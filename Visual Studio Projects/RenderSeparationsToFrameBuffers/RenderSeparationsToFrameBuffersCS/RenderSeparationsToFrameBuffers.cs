/* --------------------------------------------------------------------------------
 *  <copyright file="Program.cs" company="Global Graphics Software Ltd">
 *    Copyright (c) 2025 Global Graphics Software Ltd. All rights reserved.
 *  </copyright>
 *  <summary>
 *    This example is provided on an "as is" basis and without warranty of any kind.
 *    Global Graphics Software Ltd. does not warrant or make any representations
 *    regarding the use or results of use of this example.
 *  </summary>
 * ---------------------------------------------------------------------------------
 */

using JawsMako;

namespace RenderSeparationsToFrameBuffersCS
{
    internal class RenderSeparationsToFrameBuffers
    {
        private const string TestFilesPath = @"..\..\..\..\..\..\TestFiles\";

        static int Main(string[] args)
        {
            try
            {
                if (args.Length != 4)
                {
                    Console.Error.WriteLine($"Usage: {AppDomain.CurrentDomain.FriendlyName} <source file> <spots to retain> <spots to ignore> <framebuffers=true/false>");
                    return 1;
                }

                var inputFile = args[0];
                var spotsToRetain = args[1];
                var spotsToIgnore = args[2];
                var renderToFrameBuffers = args[3];

                if (renderToFrameBuffers != "true" && renderToFrameBuffers != "false")
                {
                    Console.Error.WriteLine("Parameter framebuffers must be true or false not " + args[3]);
                    return 1;
                }

                using var mako = IJawsMako.create();
                IJawsMako.enableAllFeatures(mako);

                // Input
                using var pdfInput = IPDFInput.create(mako);
                using var assembly = pdfInput.open(TestFilesPath + inputFile);
                using var page = assembly.getDocument().getPage();
                using var fixedPage = page.getContent();

                // Set image dimensions + colorspace
                var resolution = 576.0;
                using var bounds = new FRect(0, 0, page.getWidth(), page.getHeight());

                var pixelWidth = (uint)Math.Round(bounds.dX / 96.0 * resolution);
                var pixelHeight = (uint)Math.Round(bounds.dY / 96.0 * resolution);

                const int depth = 8;
                using var testspace = IDOMColorSpaceDeviceCMYK.create(mako);

                // Create spot color lists
                using var inks = IRendererTransform.findInks(mako, fixedPage);

                var componentNames = new List<string>();
                using var ignoreSpotColorNames = new CEDLVectString();
                using var retainSpotColorNames = new CEDLVectString();
                var numComponents = testspace.getNumComponents();

                for (var i = 0; i < numComponents; i++)
                    componentNames.Add(testspace.getColorantName((byte)i));
                
                foreach (var inkName in inks.toVector().Select(ink => ink.getInkName()))
                {
                    if (spotsToIgnore.Contains(inkName))
                        ignoreSpotColorNames.append(inkName);
                    else if (spotsToRetain.Contains(inkName))
                    {
                        retainSpotColorNames.append(inkName);
                        componentNames.Add(inkName);
                    }
                }

                using var renderer = IJawsRenderer.create(mako);

                if (renderToFrameBuffers == "false")
                {
                    // Render using renderSeparations()
                    using var images = renderer.renderSeparations(
                        fixedPage,
                        depth,
                        testspace,
                        0,
                        bounds,
                        pixelWidth,
                        pixelHeight,
                        retainSpotColorNames,
                        IOptionalContent.Null(),
                        eOptionalContentEvent.eOCEView,
                        new CEDLVectString(),
                        false,
                        0,
                        ignoreSpotColorNames);

                    // Write the outputs to TIFF files
                    var stem = Path.Combine(
                        Path.GetDirectoryName(inputFile) ?? string.Empty,
                        Path.GetFileNameWithoutExtension(inputFile)
                    );

                    for (uint j = 0; j < componentNames.Count; j++)
                    {
                        var tiffFileName = $"{stem}_regular_{componentNames[(int)j]}.tif";
                        IDOMTIFFImage.encode(mako, images[j], IOutputStream.createToFile(mako, tiffFileName));
                    }
                }
                else
                {
                    // Prepare frame buffers
                    var numChannels = componentNames.Count;
                    var sourceStride = (int)pixelWidth * numChannels;

                    var buffers = new byte[numChannels][];

                    using var frameBuffers = new CEDLVectCFrameBufferInfo();
                    for (byte bufferIndex = 0; bufferIndex < numChannels; ++bufferIndex)
                    {
                        // One byte per pixel (depth = 8, single component per plane)
                        buffers[bufferIndex] = new byte[pixelHeight * pixelWidth];

                        var frameBufferInfo = new IJawsRenderer.CFrameBufferInfo
                        {
                            bufferOfs = 0,
                            rowStride = (int)pixelWidth,
                            pixelStride = 0
                        };
                        frameBuffers.append(frameBufferInfo);
                    }

                    // Render using renderSeparationsToFrameBuffers()
                    renderer.renderSeparationsToFrameBuffers(
                        fixedPage,
                        depth,
                        /*hostEndian=*/ true,
                        pixelWidth,
                        pixelHeight,
                        testspace,
                        buffers,
                        frameBuffers,
                        0,
                        bounds,
                        new CEDLVectWString(retainSpotColorNames.toArray()),
                        IOptionalContent.Null(),
                        eOptionalContentEvent.eOCEView,
                        new CEDLVectWString(), // extra components (none)
                        /*alphaGeneration*/ false,
                        /*bandMemorySize*/ 0,
                        new CEDLVectWString(ignoreSpotColorNames.toArray())
                    );

                    // Write the outputs to TIFF files
                    var stem = Path.Combine(
                        Path.GetDirectoryName(inputFile) ?? string.Empty,
                        Path.GetFileNameWithoutExtension(inputFile)
                    );

                    for (uint j = 0; j < numChannels; j++)
                    {
                        // From frame buffers
                        var tiffFileName = $"{stem}_frameBuffer_{componentNames[(int)j]}.tif";
                        using var pair = IDOMTIFFImage.createWriterAndImage(
                            mako,
                            IDOMColorSpaceDeviceGray.create(mako),
                            pixelWidth,
                            pixelHeight,
                            depth,
                            96.0, 96.0,
                            IDOMTIFFImage.eTIFFCompression.eTCAuto,
                            IDOMTIFFImage.eTIFFPrediction.eTPNone,
                            eImageExtraChannelType.eIECNone,
                            /*tiled*/ false,
                            IInputStream.createFromFile(mako, TestFilesPath + inputFile),
                            IOutputStream.createToFile(mako, tiffFileName)
                        );

                        using var frameWriter = pair.frameWriter;

                        for (uint y = 0; y < pixelHeight; y++)
                        {
                            var scanline = new byte[sourceStride];
                            Buffer.BlockCopy(buffers[j], (int)(y * pixelWidth), scanline, 0, (int)pixelWidth);
                            frameWriter.writeScanLine(scanline);
                        }

                        frameWriter.flushData();
                    }
                }
            }
            catch (MakoException e)
            {
                Console.Error.WriteLine($"Exception thrown: {e.m_errorCode}: {e.m_msg}");
                return 1;
            }
            catch (Exception e)
            {
                Console.Error.WriteLine($"Exception thrown: {e}");
                return 1;
            }

            return 0;
        }
    }
}