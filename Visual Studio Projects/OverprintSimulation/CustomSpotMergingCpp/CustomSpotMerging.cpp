
/* -----------------------------------------------------------------------
 * <copyright file="CustomSpotMerge.cpp" company="Hybrid Software Ltd">
 *  Copyright (c) 2025 Hybrid Software Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Hybrid Software Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 * </summary>
 * -----------------------------------------------------------------------
 */

#include <filesystem>
#include <iostream>

#include <jawsmako/jawsmako.h>
#include <jawsmako/pdfoutput.h>
#include <jawsmako/pdfinput.h>

using namespace JawsMako;
using namespace EDL;

const U8String TEST_FILES_PATH = R"(..\..\..\..\TestFiles\)";

int main()
{
    try
    {
        const auto mako = IJawsMako::create();
        IJawsMako::enableAllFeatures(mako);

        // Load the document
        auto documentAssembly = IPDFInput::create(mako)->open(TEST_FILES_PATH + "Robots Plus Process Colors.pdf");
        auto document = documentAssembly->getDocument();
        for (uint32 i = 0; i < document->getNumPages(); i++)
        {
            // Set image dimensions + colorspace
            const auto fixedPage = document->getPage(i)->getContent();
            const auto bounds = FRect(0, 0, fixedPage->getWidth(), fixedPage->getHeight());
            constexpr double resolution = 150;
            const auto pixelWidth = static_cast<uint32_t>(lround(bounds.dX / 96.0 * resolution));
            const auto pixelHeight = static_cast<uint32_t>(lround(bounds.dY / 96.0 * resolution));
            const auto cmyk = IDOMColorSpaceDeviceCMYK::create(mako); // colorspace must be cmyk for spot merging (we can convert back to rgb later if necessary)

            // Get spot lists
            const auto spots = IRendererTransform::inkInfoToColorantInfo(mako, IRendererTransform::findInks(mako, fixedPage), cmyk);
            CSpotColorNames spotNames;
            for (auto& spot : spots) 
                spotNames.append(spot.name);

            // Prepare frame buffers
            const auto numProcess = cmyk->getNumComponents();
            const auto numSpots = spots.size();
            const auto numBuffers = numProcess + numSpots;
            std::vector<std::vector<uint8_t>> buffers(numBuffers);
            auto frameBuffers = IJawsRenderer::CFrameBufferInfoVect(numBuffers);
            for (unsigned bufferIndex = 0; bufferIndex < numBuffers; ++bufferIndex)
            {
                buffers[bufferIndex].resize(pixelHeight * pixelWidth);
                frameBuffers[bufferIndex].buffer = buffers[bufferIndex].data();
                frameBuffers[bufferIndex].rowStride = static_cast<int>(pixelWidth);
                frameBuffers[bufferIndex].pixelStride = 0;
            }

            // Render using renderSeparationsToFrameBuffers()
            auto renderer = IJawsRenderer::create(mako);
            renderer->renderSeparationsToFrameBuffers(
                fixedPage,
                8,
                true,
                pixelWidth,
                pixelHeight,
                cmyk,
                frameBuffers,
                0,
                bounds,
                spotNames,
                IOptionalContentPtr(),
                eOCEPrint // Non printable layers will be off
            );


            // write the buffers to an image
            IImageFrameWriterPtr frameWriter;
            auto image = IDOMRawImage::createWriterAndImage(mako, frameWriter, cmyk,
                pixelWidth, pixelHeight,
                8, resolution, resolution);

            // Get spot components 
            CEDLVector<CFloatVect> components(numSpots);
            for (uint32_t s = 0; s < numSpots; s++)
                components[s] = CFloatVect({
                    spots[s].components[0],
                    spots[s].components[1],
                    spots[s].components[2],
                    spots[s].components[3]
                    });

            // Merge spots into process
            CEDLVector<uint8_t*> outPtrs(numProcess);
            CEDLVector<uint8_t*> inPtrs(numSpots);
            constexpr float inv255 = 1.0f / 255.0f;

            for (uint32_t y = 0; y < pixelHeight; ++y)
            {
                for (uint32_t j = 0; j < numProcess; ++j)
                    outPtrs[j] = static_cast<uint8_t*>(frameBuffers[j].buffer) + y * frameBuffers[j].rowStride;

                for (uint32_t k = 0; k < numSpots; ++k)
                    inPtrs[k] = static_cast<uint8_t*>(frameBuffers[numProcess + k].buffer) + y * frameBuffers[numProcess + k].rowStride;

                auto scanline = std::vector<uint8_t>(pixelWidth * numProcess);

                for (uint32_t x = 0; x < pixelWidth; ++x)
                    for (uint32_t c = 0; c < 4; ++c)
                    {
                        scanline[x * numProcess + c] = outPtrs[c][x];
                        for (uint32_t s = 0; s < numSpots; ++s)
                        {
                            float spotVal = inPtrs[s][x] * inv255;
                            float currentVal = scanline[x * numProcess + c] * inv255;
                            float newVal = 1.0f - ((1.0f - components[s][c] * spotVal) * (1.0f - currentVal));
                            scanline[x * numProcess + c] = static_cast<uint8_t>(newVal * 255.0f + 0.5f);
                        }
                    }
                frameWriter->writeScanLine(scanline.data());
            }

            frameWriter->flushData();

            //now we can convert back to RGB if needed
            const auto filteredImage = IDOMFilteredImage::create(mako, image, IDOMImageColorConverterFilter::create(mako, IDOMColorSpaceDeviceRGB::create(mako), eRelativeColorimetric, eBPCDefault));

            // save the output to jpeg file
            auto outputJPEG = "output_" + std::to_string(i) + ".jpg";
            IDOMJPEGImage::encode(mako, static_cast<IDOMImagePtr>(filteredImage), IOutputStream::createToFile(mako, outputJPEG.c_str()));
        }
    }
    catch (IError& e)
    {
        const String errorFormatString = getEDLErrorString(e.getErrorCode());
        std::wcerr << L"Exception thrown: " << e.getErrorDescription(errorFormatString) << '\n';
        return static_cast<int>(e.getErrorCode());
    }
    catch (std::exception& e)
    {
        std::wcerr << L"std::exception thrown: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
