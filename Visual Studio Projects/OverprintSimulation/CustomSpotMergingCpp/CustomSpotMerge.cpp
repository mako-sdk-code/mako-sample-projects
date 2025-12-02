
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

#include <iostream>

#include <jawsmako/jawsmako.h>
#include <jawsmako/pdfoutput.h>
#include <jawsmako/pdfinput.h>

using namespace JawsMako;
using namespace EDL;

int main()
{

    U8String testFilePath = R"(..\..\TestFiles\)";

    try
    {
        const auto mako = IJawsMako::create();
        mako->enableAllFeatures(mako);

        // Load the document
        IDocumentAssemblyPtr documentAssembly = IPDFInput::create(mako)->open(testFilePath + "Robots Plus Process Colors.pdf");
        auto document = documentAssembly->getDocument();
        for (int i = 0; i < document->getNumPages(); i++)
        {
            // Set image dimensions + colorspace
            const auto fixedPage = document->getPage(i)->getContent();
            const auto bounds = FRect(0, 0, fixedPage->getWidth(), fixedPage->getHeight());
            const double resolution = 150;
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
            for (uint8_t bufferIndex = 0; bufferIndex < numBuffers; ++bufferIndex)
            {
                buffers[bufferIndex].resize(pixelHeight * pixelWidth);
                frameBuffers[bufferIndex].buffer = buffers[bufferIndex].data();
                frameBuffers[bufferIndex].rowStride = static_cast<int>(pixelWidth);
                frameBuffers[bufferIndex].pixelStride = 0;
            }

            // Render using renderSeparationsToFrameBuffers()
            IJawsRendererPtr renderer = IJawsRenderer::create(mako);
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
            for (uint32_t i = 0; i < numSpots; i++)
                components[i] = CFloatVect({
                    spots[i].components[0],
                    spots[i].components[1],
                    spots[i].components[2],
                    spots[i].components[3]
                    });

            // Merge spots into process
            CEDLVector<uint8_t*> outPtrs(numProcess);
            CEDLVector<uint8_t*> inPtrs(numSpots);
            constexpr float inv255 = 1.0f / 255.0f;

            for (uint32_t y = 0; y < pixelHeight; ++y)
            {
                for (uint32_t i = 0; i < numProcess; ++i)
                    outPtrs[i] = static_cast<uint8_t*>(frameBuffers[i].buffer) + y * frameBuffers[i].rowStride;

                for (uint32_t i = 0; i < numSpots; ++i)
                    inPtrs[i] = static_cast<uint8_t*>(frameBuffers[numProcess + i].buffer) + y * frameBuffers[numProcess + i].rowStride;

                auto scanline = std::vector<uint8_t>(pixelWidth * numProcess);

                for (uint32_t x = 0; x < pixelWidth; ++x)
                    for (uint32_t c = 0; c < 4; ++c)
                    {
                        scanline[x * numProcess + c] = outPtrs[c][x];
                        for (uint32_t i = 0; i < numSpots; ++i)
                        {
                            float spotVal = inPtrs[i][x] * inv255;
                            float currentVal = scanline[x * numProcess + c] * inv255;
                            float newVal = 1.0f - ((1.0f - components[i][c] * spotVal) * (1.0f - currentVal));
                            scanline[x * numProcess + c] = static_cast<uint8_t>(newVal * 255.0f + 0.5f);
                        }
                    }
                frameWriter->writeScanLine(scanline.data());
            }

            frameWriter->flushData();

            //now we can convert back to RGB if needed
            const auto filteredImage = IDOMFilteredImage::create(mako, image, IDOMImageColorConverterFilter::create(mako, IDOMColorSpaceDeviceRGB::create(mako), eRelativeColorimetric, eBPCDefault));

            // save the output to jpeg file
            std::string outputJPEG = "output_" + std::to_string(i) + ".jpg";
            IDOMJPEGImage::encode(mako, (IDOMImagePtr)filteredImage, IOutputStream::createToFile(mako, outputJPEG.c_str()));
        }
    }
    catch (IError& e)
    {
        const String errorFormatString = getEDLErrorString(e.getErrorCode());
        std::wcerr << L"Exception thrown: " << e.getErrorDescription(errorFormatString) << std::endl;
        return static_cast<int>(e.getErrorCode());
    }
    catch (std::exception& e)
    {
        std::wcerr << L"std::exception thrown: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}