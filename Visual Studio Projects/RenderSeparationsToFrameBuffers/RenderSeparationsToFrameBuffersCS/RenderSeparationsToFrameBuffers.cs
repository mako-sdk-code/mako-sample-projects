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
using System.Diagnostics;
using static JawsMako.jawsmakoIF_csharp;

namespace RenderSeparationsToFrameBuffersCS
{
    internal class RenderSeparationsToFrameBuffers
    {
        static int Main(string[] args)
        {
            try
            {
                var testFilepath = @"..\..\..\..\TestFiles\";

                if (args.Length < 4)
                {
                    Console.WriteLine($"Usage: {AppDomain.CurrentDomain.FriendlyName} <source file> <framebuffers=true/false> <spots to retain> <spots to ignore> <framebuffers=true/false");
                    return -1;
                }
                string inputFile = args[0];
                string spotsToRetain = args[1];
                string spotsToIgnore = args[2];
                string renderToFrameBuffers = args[3];

                if (renderToFrameBuffers != "true" && renderToFrameBuffers != "false")
                {
                    Console.WriteLine("Parameter framebuffers must be true or false not" + args[3]);
                    return -1;
                }

                var mako = IJawsMako.create();
                IJawsMako.enableAllFeatures(mako);

                // Input
                var pdfInput = IPDFInput.create(mako);
                using var assembly = pdfInput.open(testFilepath + inputFile);
                using var page = assembly.getDocument().getPage();
                using var fixedPage = page.getContent();

                // Set image dimensions + colorspace
                double resolution = 576.0;
                var bounds = new FRect(0, 0, page.getWidth(), page.getHeight());

                uint pixelWidth = (uint)Math.Round(bounds.dX / 96.0 * resolution);
                uint pixelHeight = (uint)Math.Round(bounds.dY / 96.0 * resolution);

                const int depth = 8;
                var testspace = IDOMColorSpaceDeviceCMYK.create(mako);

                // Create spot color lists
                var inks = IRendererTransform.findInks(mako, fixedPage);

                var componentNames = new List<string>();
                var ignoreSpotColorNames = new CEDLVectString();
                var retainSpotColorNames = new CEDLVectString();
                var numComponents = testspace.getNumComponents();

                for (int i = 0; i < numComponents; i++)
                    componentNames.Add(testspace.getColorantName((byte)i));
                
                foreach (var ink in inks.toVector())
                {
                    var inkName = ink.getInkName();

                    if (spotsToIgnore.Contains(inkName))
                        ignoreSpotColorNames.append(inkName);
                    else if (spotsToRetain.Contains(inkName))
                    {
                        retainSpotColorNames.append(inkName);
                        componentNames.Add(inkName);
                    }
                }

                var renderer = IJawsRenderer.create(mako);

                if (renderToFrameBuffers == "false")
                {
                    // Render using renderSeparations()
                    CEDLVectIDOMImage images = renderer.renderSeparations(
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
                    string stem = Path.Combine(
                        Path.GetDirectoryName(inputFile) ?? string.Empty,
                        Path.GetFileNameWithoutExtension(inputFile) ?? "output"
                    );

                    for (uint j = 0; j < componentNames.Count(); j++)
                    {
                        string tiffFileName = $"{stem}_regular_{componentNames[(int)j]}.tif";
                        IDOMTIFFImage.encode(mako, images[j], IOutputStream.createToFile(mako, tiffFileName));
                    }
                }
                else
                {
                    // Prepare frame buffers
                    int numChannels = componentNames.Count;
                    int sourceStride = (int)pixelWidth * numChannels;

                    var buffers = new byte[numChannels][];

                    var frameBuffers = new CEDLVectCFrameBufferInfo();
                    for (byte bufferIndex = 0; bufferIndex < numChannels; ++bufferIndex)
                    {
                        // One byte per pixel (depth = 8, single component per plane)
                        buffers[bufferIndex] = new byte[pixelHeight * (uint)pixelWidth];

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
                        /*alphGeneration*/ false,
                        /*bandMemorySize*/ 0,
                        new CEDLVectWString(ignoreSpotColorNames.toArray())
                    );

                    // Write the outputs to TIFF files
                    string stem = Path.Combine(
                        Path.GetDirectoryName(inputFile) ?? string.Empty,
                        Path.GetFileNameWithoutExtension(inputFile) ?? "output"
                    );

                    for (uint j = 0; j < numChannels; j++)
                    {
                        // From frame buffers
                        string tiffFileName = $"{stem}_frameBuffer_{componentNames[(int)j]}.tif";
                        var pair = IDOMTIFFImage.createWriterAndImage(
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
                            IInputStream.createFromFile(mako, testFilepath + inputFile), 
                            IOutputStream.createToFile(mako, tiffFileName)
                        );

                        IImageFrameWriter frameWriter = pair.frameWriter;

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
                Console.WriteLine($"Exception thrown: {e.m_errorCode}: {e.m_msg}");
            }
            catch (Exception e)
            {
                Console.WriteLine($"Exception thrown: {e}");
            }

            return 0;
        }
    }
}