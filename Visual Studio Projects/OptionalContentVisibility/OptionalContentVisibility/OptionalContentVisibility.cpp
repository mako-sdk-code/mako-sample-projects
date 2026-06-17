/* -----------------------------------------------------------------------
 * <copyright file="OptionalContentVisibility.cpp" company="Hybrid Software Helix Ltd">
 *  Copyright (c) 2026 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Hybrid Software Helix Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 * </summary>
 * -----------------------------------------------------------------------
 */

#include <jawsmako/jawsmako.h>
#include <jawsmako/pdfoutput.h>
#include <edl/idomglyphs.h>

using namespace JawsMako;
using namespace EDL;

int main()
{
    // --- Create a document assembly with some text ---
	auto mako = IJawsMako::create();
    IJawsMako::enableAllFeatures(mako);

    auto assembly = IDocumentAssembly::create(mako);
    auto document = IDocument::create(mako);
    auto page = IPage::create(mako);
    auto content = IDOMFixedPage::create(mako);
    content->setWidth(595);
    content->setHeight(842);

    uint32 fontIndex = 0;
    auto font = mako->findFont("Arial", fontIndex);
    auto black = IDOMSolidColorBrush::create(
        mako,
        IDOMColor::create(mako, IDOMColorSpaceDeviceRGB::create(mako), 1.0, 0.0, 0.0, 0.0));

    auto glyphBoth = IDOMGlyphs::create(mako, L"Always visible", 28.0, FPoint(72.0, 120), black, font, fontIndex);
    auto glyphView = IDOMGlyphs::create(mako, L"VIEW only", 28.0, FPoint(72.0, 170), black, font, fontIndex);
    auto glyphPrint = IDOMGlyphs::create(mako, L"PRINT only", 28.0, FPoint(72.0, 220), black, font, fontIndex);

	content->appendChild(glyphBoth);
    content->appendChild(glyphView);
    content->appendChild(glyphPrint);
    page->setContent(content);
    document->appendPage(page);
    assembly->appendDocument(document);

	// Create 3 optional content groups and associate the text with the appropriate optional content groups 
    auto optionalContent = IOptionalContent::create(mako);
    document->setOptionalContent(optionalContent);

    auto groupBoth = optionalContent->makeNewGroup("Both", true);
    auto groupView = optionalContent->makeNewGroup("View only", true);
    auto groupPrint = optionalContent->makeNewGroup("Print only", true);

    optionalContent->makeNodeOptional(glyphBoth, groupBoth);
    optionalContent->makeNodeOptional(glyphView, groupView);
    optionalContent->makeNodeOptional(glyphPrint, groupPrint);

	// Set the usage for each group to specify when the content in each group should be visible. 
    auto makeUsage = [&](const IOptionalContentGroupPtr& group,
        eOptionalContentVisibility view,
        eOptionalContentVisibility print)
        {
            auto usage = IOptionalContentGroupUsage::create(mako);
            usage->setViewVisibility(view);
            usage->setPrintVisibility(print);
            group->setUsage(usage);
        };

    makeUsage(groupBoth, eOCVVisible, eOCVVisible);
    makeUsage(groupView, eOCVVisible, eOCVInvisible);
    makeUsage(groupPrint, eOCVInvisible, eOCVVisible);

    // --- Set usage applications for the default configuration ---
    auto config = optionalContent->getDefaultConfiguration();
    config->setBaseState(eOCVVisible);

    COptionalContentGroupReferenceVect allGroups;
    allGroups.append(groupBoth->getReference());
    allGroups.append(groupView->getReference());
    allGroups.append(groupPrint->getReference());

    auto makeUsageApplication = [&](
        eOptionalContentEvent event,
        eOptionalContentCategory category)
        {
            auto app = IOptionalContentGroupUsageApplication::create(mako, event);
            COCCategoryVect categories;
            categories.append(category);
            app->setCategories(categories);
            app->setAffectedOptionalContentGroups(allGroups);
            return app;
        };

    COptionalContentGroupUsageApplicationVect apps;
    apps.append(makeUsageApplication(eOCEView, eOCCView));
    apps.append(makeUsageApplication(eOCEPrint, eOCCPrint));
    config->setAutoStates(apps);

	// --- Create a PDF and render the content to PNGs ---
    IPDFOutput::create(mako)->writeAssembly(assembly, "ocg-view-print.pdf");

    auto renderer = IJawsRenderer::create(mako);
    auto viewPng = renderer->render(
        content, 96, 8, IDOMColorSpaceDeviceRGB::create(mako),
        FRect(), false, CSpotColorNames(), optionalContent, eOCEView);

    auto printPng = renderer->render(
        content, 96, 8, IDOMColorSpaceDeviceRGB::create(mako),
        FRect(), false, CSpotColorNames(), optionalContent, eOCEPrint);

    IDOMPNGImage::encode(mako, viewPng,
        IOutputStream::createToFile(mako, "view.png"));
    IDOMPNGImage::encode(mako, printPng,
        IOutputStream::createToFile(mako, "print.png"));
}
