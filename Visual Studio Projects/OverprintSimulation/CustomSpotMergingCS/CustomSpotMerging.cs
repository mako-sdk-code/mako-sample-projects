// -----------------------------------------------------------------------
// <copyright file="CustomSpotMerge.cs" company="Hybrid Software">
//   Copyright (c) 2025 Hybrid Software.
// </copyright>
// <summary>
//   Example provided "as is", without warranty of any kind.
// </summary>
// -----------------------------------------------------------------------

using JawsMako;

class CustomSpotMergingCS
{
    private const string TestFilesPath = @"..\..\..\..\..\..\TestFiles\";

    static int Main()
    {
        try
        {
            using var mako = IJawsMako.create();
            IJawsMako.enableAllFeatures(mako);

            // Load the document
            using var assembly = IPDFInput.create(mako).open(Path.Combine(TestFilesPath, "Robots Plus Process Colors.pdf"));
            using var document = assembly.getDocument();

            for (uint pageIndex = 0; pageIndex < document.getNumPages(); pageIndex++)
            {
                using var fixedPage = document.getPage(pageIndex).getContent();

                // Page bounds & raster geometry
                using var bounds = new FRect(0, 0, fixedPage.getWidth(), fixedPage.getHeight());
                const double resolution = 150.0;
                var pixelWidth = (uint)Math.Round(bounds.dX / 96.0 * resolution);
                var pixelHeight = (uint)Math.Round(bounds.dY / 96.0 * resolution);

                // Colorspace must be CMYK for spot merging (we can convert to RGB later)
                using var cmyk = IDOMColorSpaceDeviceCMYK.create(mako);

                // ----- Find inks and build spot list (names + CMYK components) -----
                using var spots = IRendererTransform.inkInfoToColorantInfo(mako, IRendererTransform.findInks(mako, fixedPage), cmyk);
                using var spotNames = new CEDLVectWString();

                foreach (var spot in spots.toVector())
                    spotNames.append(spot.name);

                int numProcess = cmyk.getNumComponents(); // 4 (CMYK)
                var numSpots = (int)spots.size();
                var numBuffers = numProcess + numSpots;

                // ----- Prepare frame buffers for renderSeparationsToFrameBuffers() -----
                var buffers = new byte[numBuffers][];
                using var fb = new CEDLVectCFrameBufferInfo();

                for (var i = 0; i < numBuffers; i++)
                {
                    buffers[i] = new byte[pixelHeight *  pixelWidth];

                    var info = new IJawsRenderer.CFrameBufferInfo
                    {
                        bufferOfs = 0,
                        rowStride = (int)pixelWidth,   // pixels per row (1 byte per pixel)
                        pixelStride = 0                // tightly packed
                    };
                    fb.append(info);
                }

                // ----- Render true separations (process first, then spots) -----
                using var renderer = IJawsRenderer.create(mako);
                renderer.renderSeparationsToFrameBuffers(
                    fixedPage,
                    8,                    // bits per component per plate
                    true,                 // antialiased
                    pixelWidth,
                    pixelHeight,
                    cmyk,                 // process space -> first 4 plates (C,M,Y,K)
                    buffers,
                    fb,
                    0,                    // bandMemorySize (0 = no banding)
                    bounds,
                    spotNames,            // which spots to keep (all found)
                    IOptionalContent.Null(),
                    eOptionalContentEvent.eOCEPrint // Non-printable layers will be off
                );

                // ----- Create writer and image -----
                using var pair = IDOMRawImage.createWriterAndImage(
                    mako,
                    cmyk,
                    pixelWidth,
                    pixelHeight,
                    8,
                    resolution, resolution
                );

                using var frameWriter = pair.frameWriter;

                // ----- Get spot components of spots to merge -----
                using var components = new CEDLVectVectFloat();
                for (uint i = 0; i < numSpots; i++)
                    components.append(new CEDLVectFloat(new StdVectFloat()
                    {
                        spots[i].components[0],
                        spots[i].components[1],
                        spots[i].components[2],
                        spots[i].components[3]
                    }));

                // ----- Merge each spot component with the process buffer values and write the result to the scanline -----
                const float inv255 = 1.0f / 255.0f;
                for (uint y = 0; y < pixelHeight; y++)
                {
                    var rowStart = (int)(y * pixelWidth);
                    var scanline = new byte[pixelWidth * numProcess];

                    for (uint x = 0; x < pixelWidth; ++x)
                    {
                        for (uint c = 0; c < numProcess; ++c)
                        {
                            scanline[x * numProcess + c] = buffers[c][rowStart + x];
                            for (uint i = 0; i < numSpots; ++i)
                            {
                                var spotVal = buffers[numProcess + i][rowStart + x] * inv255;
                                var currentVal = scanline[x * numProcess + c] * inv255;
                                var newVal = 1.0f - (1.0f - components[i][c] * spotVal) * (1.0f - currentVal);
                                scanline[x * numProcess + c] = (byte)(newVal * 255.0f + 0.5f);
                            }
                        }
                    }
                    frameWriter.writeScanLine(scanline);
                }

                frameWriter.flushData();

                // ----- Convert to RGB (optional) and save as a JPEG (or other image type) -----
                using var rgb = IDOMColorSpaceDeviceRGB.create(mako);
                using var cc = IDOMImageColorConverterFilter.create(mako, rgb, eRenderingIntent.eRelativeColorimetric, eBlackPointCompensation.eBPCDefault);
                using var filtered = IDOMFilteredImage.create(mako, pair.domImage, cc);

                var outJpeg = $"output_{pageIndex}.jpg";
                IDOMJPEGImage.encode(mako, filtered, IOutputStream.createToFile(mako, outJpeg));
                Console.WriteLine($"Wrote: {outJpeg}");
            }
        }
        catch (MakoException me)
        {
            Console.Error.WriteLine($"Mako error {me.m_errorCode}: {me.m_msg}");
            return 1;
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine(ex);
            return 1;
        }

        return 0;
    }
}