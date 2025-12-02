
/* -----------------------------------------------------------------------
 * <copyright file="Main.cpp" company="Global Graphics Software Ltd">
 *  Copyright (c) 2025 Global Graphics Software Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Global Graphics Software Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 * </summary>
 * -----------------------------------------------------------------------
 */

#include <iostream>

#include <jawsmako/jawsmako.h>
#include <jawsmako/pdfoutput.h>

using namespace JawsMako;
using namespace EDL;

int main(int argc, char* argv[])
{

    U8String testFilePath = R"(..\..\TestFiles\)";
    if (argc < 4)
    {
        std::wcerr << "Usage: " << argv[0] << " <source file> <spots to retain> <spots to ignore> <framebuffers=true/false>] " << std::endl;
        return -1;
    }
    U8String inputFile = argv[1];
    U8String spotsToRetain = argv[2];
    U8String spotsToIgnore = argv[3];
    U8String renderToFrameBuffers = argv[4];
    
    if (renderToFrameBuffers != "true" && renderToFrameBuffers != "false")
    {
        std::wcerr << "Parameter framebuffers must be either true or false not " << argv[4] << std::endl;
        return -1;
    }
        

    try
    {
        const auto mako = IJawsMako::create();
        mako->enableAllFeatures(mako);
        const auto assembly = IInput::create(mako, eFFPDF)->open(testFilePath + inputFile);
        const auto document = assembly->getDocument();
        const auto page = document->getPage(0);
        const auto fixedPage = page->edit();
        
        // Set image dimensions + colorspace
        double resolution = 576.0;
        const auto bounds = FRect(0, 0, page->getWidth(), page->getHeight());
        auto pixelWidth = static_cast<uint32_t>(lround(bounds.dX / 96.0 * resolution));
        auto pixelHeight = static_cast<uint32_t>(lround(bounds.dY / 96.0 * resolution));
        const auto depth = 8;
        auto testspace = IDOMColorSpaceDeviceCMYK::create(mako);

        // Create spot color lists
        const auto inks = IRendererTransform::findInks(mako, fixedPage);
        CU8StringVect componentNames;
        CSpotColorNames ignoreSpotColorNames;
        CSpotColorNames retainSpotColorNames;

        for (auto i = 0; i < testspace->getNumComponents(); i++)
        {
            componentNames.append(testspace->getColorantName(i));
        }

        for (const auto& ink : inks)
        {
            auto inkName = ink.inkName;
            if (spotsToIgnore.contains(inkName))
            {
                ignoreSpotColorNames.append(inkName);
            }
            else if (spotsToRetain.contains(inkName))
            {
                retainSpotColorNames.append(ink.inkName);
                componentNames.append(ink.inkName);
            }
            else 
                continue; // If a spot color is not specified in either list it will be represented in the process separations by default, so we do not need to do anything
        }
        
        // Render using renderSeparations()
        auto render = IJawsRenderer::create(mako);

        if (renderToFrameBuffers == "false")
        {
            CEDLVector<IDOMImagePtr> images = render->renderSeparations(fixedPage, depth, testspace, true, bounds, pixelWidth, pixelHeight, retainSpotColorNames, IOptionalContentPtr(), eOCEView, CU8StringVect(), false, 0, ignoreSpotColorNames);
            
            for (uint32 j = 0; j < componentNames.size(); j++)
            {
                EDLSysString tiffFileName = std::format("{0}_regular_{1}.tif", inputFile.substr(0, inputFile.size() - 4), componentNames[j]).c_str();
                IDOMTIFFImage::encode(mako, images[j], IOutputStream::createToFile(mako, tiffFileName));
            }
        }
        else
        {
            // Prepare frameBuffers
            auto numComponents = componentNames.size();
            std::vector<std::vector<uint8_t>> buffers(numComponents);
            auto frameBuffers = IJawsRenderer::CFrameBufferInfoVect(numComponents);
            for (uint8_t bufferIndex = 0; bufferIndex < numComponents; ++bufferIndex)
            {
                buffers[bufferIndex].resize(pixelHeight * pixelWidth);
                frameBuffers[bufferIndex].buffer = buffers[bufferIndex].data();
                frameBuffers[bufferIndex].rowStride = static_cast<int>(pixelWidth);
                frameBuffers[bufferIndex].pixelStride = 0;
            }

            // Render using renderSeparationsToFrameBuffers()
            render->renderSeparationsToFrameBuffers(fixedPage, depth, true, pixelWidth, pixelHeight, testspace, frameBuffers, 0, bounds, retainSpotColorNames, IOptionalContentPtr(), eOCEView, CU8StringVect(), false, 0, ignoreSpotColorNames);

            // write the outputs to tiff files to compare
            for (uint32 j = 0; j < numComponents; j++)
            {
                IImageFrameWriterPtr frame;
                EDLSysString tiffFileName = std::format("{0}_frameBuffer_{1}.tif", inputFile.substr(0, inputFile.size() - 4), componentNames[j]).c_str();
                (void)IDOMTIFFImage::createWriterAndImage(mako, frame, IDOMColorSpaceDeviceGray::create(mako),
                    pixelWidth, pixelHeight,
                    depth, 96.0, 96.0,
                    IDOMTIFFImage::eTCAuto,
                    IDOMTIFFImage::eTPNone,
                    eIECNone, false,
                    IInputStream::createFromFile(mako, tiffFileName),
                    IOutputStream::createToFile(mako, tiffFileName));
                for (uint32 y = 0; y < pixelHeight; y++)
                {
                    frame->writeScanLine(&buffers[j][y * pixelWidth * depth / 8]);
                }
                frame->flushData();
            }
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
