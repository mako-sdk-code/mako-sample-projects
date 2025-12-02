/* -----------------------------------------------------------------------
 * <copyright file="MakoRecolorImageWithSpot.cpp" company="Hybrid Software Helix Ltd">
 *  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  Recolor Image With Spot Example
 *  Demonstrates how to paint an image with a spot color.
 *
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Hybrid Software Helix Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 * </summary>
 * -----------------------------------------------------------------------
 */

#include <jawsmako/jawsmako.h>
#include <jawsmako/pdfoutput.h>
#include <edl/idomcolorspace.h>
#include <iostream>

using namespace JawsMako;
using namespace EDL;

// Helper to create an L*a*b spot color
IDOMColorPtr MakeSeparationColor(IJawsMakoPtr mako, EDLRawString name, const std::initializer_list<double>& lab)
{
    // Create a vector of colorants with one entry
    IDOMColorSpaceDeviceN::CColorantInfo colorantInfo(name, lab);
    IDOMColorSpaceDeviceN::CColorantInfoVect colorants;
    colorants.append(colorantInfo);

    // Define LAB alternate color space
    IDOMColorSpaceLABPtr alternate = IDOMColorSpaceLAB::create(mako,
        0.9642f, 1.0000f, 0.8249f,
        0.0f, 0.0f, 0.0f,
        -128.0f, 127.0f,
        -128.0f, 127.0f);

    // Create DeviceN color space with alternate
    IDOMColorSpaceDeviceNPtr colorSpace = IDOMColorSpaceDeviceN::create(
        mako->getFactory(),
        colorants,
        edlobj2IDOMColorSpace(alternate));

    // Return the new spot color
    float components[1] = { 1.0f };
    return IDOMColor::createFromArray(mako->getFactory(), colorSpace, 1.0f, components);
}

int wmain()
{
    try
    {
        const std::wstring testFilepath = L"..\\..\\TestFiles\\";

        // Instantiate Mako
        IJawsMakoPtr mako = IJawsMako::create();
        IJawsMako::enableAllFeatures(mako);

        // Create a spot color and solid color brush
        auto rubineRed = MakeSeparationColor(mako, "Pantone Rubine Red C", { 43.92, 79.0, 13.0 });
        auto rubineRedBrush = IDOMSolidColorBrush::create(mako, rubineRed);

        // Load an image from disk
        auto image = IDOMPNGImage::create(mako, IInputStream::createFromFile(mako, (testFilepath + L"Cheshire Cat.png").c_str()));

        // Get image details
        auto imageFrame = image->getImageFrame(mako);
        auto imageColorSpace = imageFrame->getColorSpace();
        double width = imageFrame->getWidth();
        double height = imageFrame->getHeight();
        double xres = imageFrame->getXResolution();
        double yres = imageFrame->getYResolution();
        FRect imageBounds(0.0, 0.0, width / xres * 96.0, height / yres * 96.0);

        // Convert to grayscale and invert
        CDOMImageFilterVect filters;
        if (imageColorSpace->getNumComponents() != 1)
        {
            filters.append(IDOMImageColorConverterFilter::create(
                mako,
                IDOMColorSpaceDeviceGray::create(mako),
                eRenderingIntent::ePerceptual,
                eBlackPointCompensation::eBPCDefault));
        }
        filters.append(IDOMImageInverterFilter::create(mako));

        // Apply filters to create a mask
        auto mask = IDOMFilteredImage::create(mako, image, filters);

        // Create a masked brush
        auto maskedBrush = IDOMMaskedBrush::create(mako, mask, rubineRedBrush, imageBounds, imageBounds);

        // Create a path and fill with masked brush
        auto geometry = IDOMPathGeometry::create(mako, imageBounds);
        auto path = IDOMPathNode::createFilled(mako, geometry, maskedBrush);

        // Create a document, page, and fixed page
        auto assembly = IDocumentAssembly::create(mako);
        auto document = IDocument::create(mako);
        assembly->appendDocument(document);
        auto page = IPage::create(mako);
        document->appendPage(page);
        auto fixedPage = IDOMFixedPage::create(mako);
        page->setContent(fixedPage);

        // Scale image to page
        path->setRenderTransform(FMatrix(imageBounds, page->getCropBox()));

        // Add to page
        fixedPage->appendChild(path);

        // Save to PDF
        IPDFOutput::create(mako)->writeAssembly(assembly, IOutputStream::createToFile(mako, L"Cheshire Cat.pdf"));
    }
    catch (IError& e)
    {
        const String errorFormatString = getEDLErrorString(e.getErrorCode());
        std::wcerr << L"Exception thrown: " << e.getErrorDescription(errorFormatString) << std::endl;
        return static_cast<int>(e.getErrorCode());
    }
    catch (const std::exception& e)
    {
        std::wcerr << L"Exception thrown: " << e.what() << std::endl;
    }

    return 0;
}
