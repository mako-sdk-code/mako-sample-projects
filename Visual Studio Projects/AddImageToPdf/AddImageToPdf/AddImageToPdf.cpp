
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

int main()
{

    U8String testFilePath = R"(..\..\TestFiles\)";

    try
    {
        const auto mako = IJawsMako::create();
        mako->enableAllFeatures(mako);
        const auto assembly = IDocumentAssembly::create(mako);
        const auto document = IDocument::create(mako);
        assembly->appendDocument(document);
        const auto page = IPage::create(mako);
        document->appendPage(page);
        const auto fixedPage = IDOMFixedPage::create(mako);
        page->setContent(fixedPage);

        const auto image = IDOMPNGImage::create(mako, IInputStream::createFromFile(mako, testFilePath + "Cheshire Cat.png"));

        // Get image attributes
        const IImageFramePtr imageFrame = image->getImageFrame(mako);
        const double width = imageFrame->getWidth() / imageFrame->getXResolution() * 96.0;
        const double height = imageFrame->getHeight() / imageFrame->getYResolution() * 96.0;

        const auto imageNode = IDOMPathNode::createImage(mako, image, FRect(0.0, 0.0, width, height));
        fixedPage->appendChild(imageNode);

        IPDFOutput::create(mako)->writeAssembly(assembly, "test.pdf");
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