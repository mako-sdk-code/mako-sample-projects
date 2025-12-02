
/* -----------------------------------------------------------------------
 * <copyright file="Main.cpp" company="Hybrid Software Helix Ltd">
 *  Copyright (c) 2025 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Hybrid Software Helix Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 * </summary>
 * -----------------------------------------------------------------------
 */

#include <iostream>
#include <format>
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;

#include <jawsmako/jawsmako.h>
#include <jawsmako/layout.h>
#include <jawsmako/outputintent.h>
#include <jawsmako/pdfinput.h>
#include <jawsmako/pdfoutput.h>
#include <edl/idomresources.h>

using namespace JawsMako;
using namespace EDL;

#define M2X(value) ((value) / 25.4 * 96.0)
#define P2X(value) ((value) / 72.0 * 96.0)

IDOMImagePtr getImage(const IJawsMakoPtr& mako, const U8String& imageFile);

int main()
{
    try
    {
        const auto mako = IJawsMako::create();
        mako->enableAllFeatures(mako);

        // Create assembly, document, page, fixed page
        const auto assembly = IDocumentAssembly::create(mako);
        const auto document = IDocument::create(mako);
        assembly->appendDocument(document);
        const auto page = IPage::create(mako);
        document->appendPage(page);
        const auto fixedPage = IDOMFixedPage::create(mako, M2X(250), M2X(210));
        page->setContent(fixedPage);

        // The ICC profiles
        CEDLVector<std::pair<U8String, U8String>> profiles;
        profiles.append(std::pair<U8String, U8String>("Probev2_ICCv4.icc", "https://www.color.org/probeprofile.xalter"));
        profiles.append(std::pair<U8String, U8String>("GRACoL2006_Coated1v2.icc", "https://idealliance.org"));
        profiles.append(std::pair<U8String, U8String>("JapanColor2011Coated.icc", "https://www.xrite.com"));

        // Get a font
        uint32 fontIndex;
        const auto font = edlobj2IDOMFontOpenType(mako->findFont("Arial", fontIndex));

        // Create a colour
        const auto darkBlue = IDOMColor::createSolidRgb(mako, 0.0f, 0.0f, 0.5f);

        // Create a layout
        const auto layout = ILayout::create(mako);

        // Add frame to hold content
        layout->addFrame(ILayoutFrame::create(FRect(M2X(12), M2X(12), M2X(273), M2X(186))));

        // Create template paragraphs
        const auto header = ILayoutParagraph::create(ILayoutParagraph::eHALeft, P2X(6));
        const auto body = ILayoutParagraph::create(ILayoutParagraph::eHALeft, P2X(6));

        // A vector to point to each paragraph to be added to the frame(s)
        auto paragraphs = CEDLVector<ILayoutParagraphPtr>();
        uint32 paraIndex = 0;

        // Create paragraphs and text runs 
        std::string text = "This PDF has an Output Intent referencing this ICC profile:";
        auto run = ILayoutTextRun::create(text.c_str(), font, fontIndex, P2X(12), darkBlue);
        paragraphs.append(header->clone());
        paragraphs[paraIndex]->addRun(run);
        text = std::format("  ● {} (from {})", profiles[0].first, profiles[0].second);
        run = ILayoutTextRun::create(text.c_str(), font, fontIndex, P2X(10));
        paragraphs.append(body->clone());
        paragraphs[++paraIndex]->addRun(run);

        // Picture
        const auto parrot = getImage(mako, R"(..\..\TestFiles\Parrot.png)");
        paragraphs.append(ILayoutParagraph::create());
        paragraphs[++paraIndex]->addRun(ILayoutImageRun::create(mako, parrot, 0, M2X(169)));

        // Add content to the page
        fixedPage->appendChild(layout->layout(paragraphs));

        // Load profile from local storage
        const auto iccProfile = IDOMICCProfile::create(mako, IInputStream::createFromFile(mako, R"(..\..\TestFiles\)" + profiles[0].first));

        // Output intent metadata
        U8String subtype = "GTS_PDFX";
        U8String registryName = "http://www.color.org";
        U8String outputCondition = profiles[0].first;
        U8String outputConditionIdentifier = profiles[0].first;
        U8String info = "Output Intent test";

        // Create a PDF output set to write PDF/X
        auto output = IPDFOutput::create(mako);
        output->setPreset("PDF/X-4");

        // Create an intent and add to PDF (the primary, and only intent recognized for PDF/X purposes)
        auto outputIntent = IOutputIntent::create(mako, subtype, outputCondition, outputConditionIdentifier, registryName, info, iccProfile);
        output->setOutputIntent(outputIntent);

        // Write PDF/X-compliant PDF
        output->writeAssembly(assembly, "OutputIntentExample.pdf");

        // Check that the PDF has an output intent
        auto testDocument = IPDFInput::create(mako)->open("OutputIntentExample.pdf")->getDocument();
        _ASSERT(testDocument->getOutputIntents().size() == 1);

        // Demonstration of adding more than one output intent. PDF/X allows it, may suit some custom workflows
        COutputIntentVect outputIntents;
        outputIntents.append(outputIntent);

        // We add two alternates
        subtype = "GGS_DUMMY";
        registryName = "https://idealliance.org";
        outputCondition = profiles[1].first;
        outputConditionIdentifier = profiles[1].first;
        info = "The GRACoL 2006 Coated v2 from idealliance.org";

        // Create an intent and add to list
        outputIntent = IOutputIntent::create(mako, subtype, outputCondition, outputConditionIdentifier, registryName, info, iccProfile);
        outputIntents.append(outputIntent);

        subtype = "GGS_DUMMY";
        registryName = "https://www.xrite.com";
        outputCondition = profiles[2].first;
        outputConditionIdentifier = profiles[2].first;
        info = "The Japan Color 2011 profile from xrite.com";

        // Create an intent and add to list
        outputIntent = IOutputIntent::create(mako, subtype, outputCondition, outputConditionIdentifier, registryName, info, iccProfile);
        outputIntents.append(outputIntent);

        // Set multiple intents
        output->setOutputIntents(outputIntents);

        // Write PDF
        output->writeAssembly(assembly, "OutputIntentExampleThreeIntents.pdf");

        // Check that the PDF has three output intents
        testDocument = IPDFInput::create(mako)->open("OutputIntentExampleThreeIntents.pdf")->getDocument();
        _ASSERT(testDocument->getOutputIntents().size() == 3);

        // Done
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

// Get image 
IDOMImagePtr getImage(const IJawsMakoPtr& mako, const U8String& imageFile)
{
    IDOMImagePtr image = IDOMImagePtr();
    const auto imagePath = fs::path(imageFile);
    if (!exists(imagePath))
    {
        std::string msg = "Image file ";
        msg += imageFile;
        msg += " not found.";
        throw std::exception(msg.c_str());
    }

    if (imagePath.extension() == ".jpg")
        image = IDOMJPEGImage::create(mako, IInputStream::createFromFile(mako, imageFile));

    if (imagePath.extension() == ".png")
        image = IDOMPNGImage::create(mako, IInputStream::createFromFile(mako, imageFile));

    if (imagePath.extension() == ".tif")
        image = IDOMTIFFImage::create(mako, IInputStream::createFromFile(mako, imageFile));

    if (!image)
    {
        std::string msg = "Image file ";
        msg += imageFile;
        msg += " could not be loaded.";
        throw std::exception(msg.c_str());
    }

    return image;
}