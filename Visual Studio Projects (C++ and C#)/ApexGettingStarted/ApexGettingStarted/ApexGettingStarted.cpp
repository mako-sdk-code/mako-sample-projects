/* -----------------------------------------------------------------------
 * <copyright file="ApexGettingStarted.cpp" company="Hybrid Software Helix Ltd">
 *  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Hybrid Software Helix Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 * </summary>
 * -----------------------------------------------------------------------
 */

#include <iostream>

#include <jawsmako/jawsmako.h>
#include <jawsmako/apexrenderer.h>

using namespace JawsMako;
using namespace EDL;

int main(int argc, const char* argv[])
{
    try
    {
        // Create our JawsMako instance.
        IJawsMakoPtr jawsMako = IJawsMako::create();
        IJawsMako::enableAllFeatures(jawsMako);

        U8String testFilePath = R"(..\..\TestFiles\)";
        U8String resultFilePath = R"(..\..\TestFiles\)";

        // Open the document assembly, the first document, the first page and get the content thereof
        auto input = IInput::create(jawsMako, eFFPDF);
        auto assembly = input->open(testFilePath + "Cheshire Cat.pdf");
        auto document = assembly->getDocument();
        auto page = document->getPage(0);
        auto cropBox = page->getCropBox();
        auto content = page->getContent();

        // Let's render with Apex
        auto renderer = IApexRenderer::create(jawsMako);

        // First we need a renderspec, in this case to render to an IDOMImage
        auto imageRenderSpec = CImageRenderSpec();

        // We want a 300dpi result, so calculate the size in pixels
        imageRenderSpec.width = static_cast<uint32_t>(page->getWidth() / 96.0 * 300.0);
        imageRenderSpec.height = static_cast<uint32_t>(page->getHeight() / 96.0 * 300.0);

        // Specify the area to be rendered
        imageRenderSpec.sourceRect = cropBox;

        // And the color space
        imageRenderSpec.processSpace = IDOMColorSpaceDeviceRGB::create(jawsMako);

        // Now render
        renderer->render(content, &imageRenderSpec);

        // Fetch the result and encode in an image
        auto image = imageRenderSpec.result;
        IDOMPNGImage::encode(jawsMako, image, IOutputStream::createToFile(jawsMako, resultFilePath + "Cheshire Cat.png"));

        return 0;
    }
    catch (IError& e)
    {
        String errorFormatString = getEDLErrorString(e.getErrorCode());
        std::wcerr << L"Exception thrown: " << e.getErrorDescription(errorFormatString) << std::endl;
        return 1;
    }
    catch (std::exception& e)
    {
        std::wcerr << L"std::exception thrown: " << e.what() << std::endl;
        return 1;
    }
}
