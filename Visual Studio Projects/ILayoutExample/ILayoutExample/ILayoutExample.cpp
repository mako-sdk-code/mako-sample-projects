/* -----------------------------------------------------------------------
 * <copyright file="ILayoutExample.cpp" company="Hybrid Software Helix Ltd">
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
#include <filesystem>
#include <cassert>
#include <jawsmako/jawsmako.h>
#include <jawsmako/layout.h>
#include <jawsmako/pdfoutput.h>
#include <edl/idommetadata.h>

using namespace std;
namespace fs = std::filesystem;
using namespace JawsMako;
using namespace EDL;

typedef std::pair<IDOMFontOpenTypePtr, uint32> otf;

#define MM2XPS(value) ((value) / 25.4 * 96.0)
#define PT2XPS(value) ((value) / 72.0 * 96.0)

// Get font
otf getOpenTypeFont(const IJawsMakoPtr& mako, const std::vector<U8String>& fontsToTry)
{
    // Pick a font
    IDOMFontPtr font;
    uint32 fontIndex = 0; // In case the font is inside a TrueType collection
    for (const auto& fontToTry : fontsToTry)
    {
        try
        {
            font = mako->findFont(fontToTry, fontIndex);
            break;
        }
        catch (IEDLError&)
        {
            // Bad or missing font - default to Arial
            font = mako->findFont("Arial", fontIndex);
        }
    }
    if (font->getFontType() == IDOMFont::eFontTypeOpenType)
        return otf{ edlobj2IDOMFontOpenType(font), fontIndex };

    return otf{ nullptr, 0 };
}

// Draw a path using a solid color brush with a thickness of 1
IDOMPathNodePtr drawFrame(const IJawsMakoPtr& mako, const FRect& frameBounds)
{
    const double margin = 4;
    const auto border = FRect(frameBounds.x - margin, frameBounds.y - margin, frameBounds.dX + margin * 2, frameBounds.dY + margin * 2);
    const auto solidBrush = IDOMSolidColorBrush::createSolidRgb(mako, 0.8f, 0.8f, 0.8f);
    const auto path = IDOMPathNode::createStroked(mako, IDOMPathGeometry::create(mako, border), solidBrush);
    path->setStrokeThickness(1);
    return path;
}

// Add a layout, draw an outline if requested
void addFrame(const IJawsMakoPtr& mako, const ILayoutPtr& layout, const IDOMFixedPagePtr& fixedPage, FRect positionAndSize, bool drawOutline = false)
{
    layout->addFrame(ILayoutFrame::create(positionAndSize));

    // Draw a box where the frame is going to be placed
    if (drawOutline)
        fixedPage->appendChild(drawFrame(mako, positionAndSize));
}

// Create paragraph
ILayoutParagraphPtr createParagraph(double spaceAfter = 0.0,
    ILayoutParagraph::eHorizontalAlignment justification = ILayoutParagraph::eHALeft,
    double leading = 1.0,
    double spaceBefore = 0.0)
{
    const auto templateParagraph = ILayoutParagraph::create(justification);
    if (spaceAfter > 0.0)
        templateParagraph->setSpacing(spaceAfter);
    if (spaceBefore > 0.0)
        templateParagraph->setSpacing(spaceBefore, true);
    templateParagraph->setLeading(leading);
    return templateParagraph;
}

// Get image and scale proportionately to specified width and/or height
IDOMImagePtr getImage(const IJawsMakoPtr& mako, const U8String& imageFile,
    double& width, double& height)
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

    const auto frame = image->getImageFrame(mako);
    const double imageWidth = frame->getWidth();
    const double imageHeight = frame->getHeight();
    const double aspectRatio = imageWidth / imageHeight;

    // If neither dimensions have been set then return the image with its 1:1 pixel dimensions
    if (width == 0.0 && height == 0.0)
    {
        width = imageWidth;
        height = imageHeight;
        return image;
    }

    // If both dimensions have been set then we're done
    if (width > 0.0 && height > 0.0)
        return image;

    // Calculate the missing dimension
    if (height == 0.0)
        height = width / aspectRatio;
    else
        width = height * aspectRatio;

    return image;
}

int main(int argc, char** argv)
{
    U8String lorem = "\nLorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.\nUt enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
    U8String testFilePath = R"(..\..\TestFiles\)";
    U8String inputFile = argc > 1 ? argv[1] : "";
    
    try
    {
        const auto mako = IJawsMako::create();
        IJawsMako::enableAllFeatures(mako);

        // Get a page ready to accept some DOM
        const auto assembly = IDocumentAssembly::create(mako);
        const auto document = IDocument::create(mako);
        assembly->appendDocument(document);
        const auto page = IPage::create(mako);
        document->appendPage(page);
        const auto fixedPage = IDOMFixedPage::create(mako);
        page->setContent(fixedPage);

        // Create a layout
        const auto layout = ILayout::create(mako);

        // Add some frames to hold content
        bool drawBorder = false;
        const uint32 margin = MM2XPS(12);
        const auto widthWithMargins = fixedPage->getWidth() - margin * 2;
        addFrame(mako, layout, fixedPage, FRect(margin, margin, widthWithMargins, MM2XPS(40)), drawBorder);                                                // Banner
        addFrame(mako, layout, fixedPage, FRect(margin, margin + MM2XPS(40), widthWithMargins / 3 - MM2XPS(2), MM2XPS(83)), drawBorder);                   // Sidebar with 2mm right margin
        addFrame(mako, layout, fixedPage, FRect(margin + widthWithMargins / 3, margin + MM2XPS(40), widthWithMargins / 3 * 2, MM2XPS(83)), drawBorder);    // Pic
        addFrame(mako, layout, fixedPage, FRect(margin, margin + MM2XPS(125), widthWithMargins, MM2XPS(155)), drawBorder);                                 // Body

        // Get a font
        const auto titleFont = getOpenTypeFont(mako, { "Arial Black" });
        assert(titleFont.first);
        const auto bodyFont = getOpenTypeFont(mako, { "DejaVu Sans Book", "Gill Sans", "Arial" });
        assert(bodyFont.first);
        const auto bodyBold = getOpenTypeFont(mako, { "DejaVu Sans Book Bold", "Gill Sans Bold", "Arial Bold" });
        assert(bodyBold.first);

        // Create a colour
        const auto darkBlue = IDOMColor::createSolidRgb(mako, 0.0f, 0.0f, 0.5f);

        CEDLSysStringVect paragraphCopy;

        paragraphCopy.append("Travel Blog");
        paragraphCopy.append("\xAThe beauty of where I found myself inspired me to write this.");
        paragraphCopy.append("As the sun rose over the horizon, casting a warm golden hue across the landscape, a breathtaking scene unfolded before the onlooker's eyes. Standing at the water's edge, one's gaze extended out over a pristine lake that shimmered like a sheet of glass, reflecting the majestic beauty that surrounded it. The lake seemed to hold its breath, mirroring with utmost precision the awe-inspiring sight that lay just beyond its tranquil surface.");
        paragraphCopy.append("Stretching magnificently into the distance, a range of rocky mountains dominated the backdrop. Each peak soared towards the heavens, their rugged surfaces etched by the passage of time and the forces of nature. The mountains stood resolute, a testament to the immense power and grandeur of the natural world. Their colors shifted subtly, painted with a breathtaking array of earthy tones \xE2\x80\x94 from deep siennas and ochres to soft grays and greens \xE2\x80\x94 all framed by the azure expanse of the sky.");
        paragraphCopy.append("The sky itself seemed to be a canvas of its own, an ever-changing masterpiece of color and light. Towering cumulus clouds danced gracefully, casting dramatic shadows that gently caressed the mountains' slopes. The fluffy white clouds looked as though they were soft pillows, inviting the weary soul to rest upon their tender embrace. The sky stretched boundlessly, seeming to touch the very edges of the earth, a reminder of the vastness of the universe and the limitless possibilities that lay beyond the human imagination.");
        paragraphCopy.append("As a gentle breeze whispered through the air, ripples formed across the surface of the lake, momentarily distorting the mirror-like reflection. The tiny waves moved in rhythmic harmony, lending an animated quality to the otherwise still waters. With each passing gust, the mountains appeared to ripple across the lake's surface, as though a magnificent magic spell had been cast upon the land.");
        paragraphCopy.append("In the distance, a lone boat glided silently across the lake, its presence adding a touch of serenity to the already tranquil scene. The boat's wake created a delicate trail on the water, a fleeting mark of human existence in the midst of the grandeur of nature. It served as a reminder of the delicate balance between mankind and the Earth, and how nature's beauty can be both admired and preserved.");
        paragraphCopy.append("Birds soared overhead, their graceful silhouettes weaving intricate patterns against the sky. They navigated the currents with effortless grace, adding life to the enchanting tableau. The soft cries of the birds mixed harmoniously with the gentle lapping of the lake's water against the shore, creating a soothing symphony that echoed through the air.");
        paragraphCopy.append("");

        // A vector to point to each paragraph to be added to the frame(s)
        CLayoutParagraphVect paragraphs;
        uint32 paraIndex = 0;

        // Title
        paragraphs.append(createParagraph(0, ILayoutParagraph::eHACenter, 1, 0));
        auto run = ILayoutTextRun::create(paragraphCopy[0], titleFont.first, titleFont.second, PT2XPS(60));
        run->setColor(darkBlue);
        paragraphs[paraIndex]->addRun(run);

        // Intro
        paragraphs.append(createParagraph(9, ILayoutParagraph::eHAJustified, 1.3));
        paragraphs[++paraIndex]->addRun(
            ILayoutTextRun::create(paragraphCopy[1], bodyBold.first, bodyBold.second, 14.0));

        // Side bar
        paragraphs.append(createParagraph(7, ILayoutParagraph::eHAJustified, 1.2));
        paragraphs[++paraIndex]->addRun(
            ILayoutTextRun::create(paragraphCopy[2], bodyFont.first, bodyFont.second, 14.0));

        // Break to next frame
        paragraphs.append(ILayoutParagraph::create());
        paragraphs[++paraIndex]->addRun(ILayoutTextRun::create("\xA", bodyFont.first, bodyFont.second, 4));

        // Picture
        if (!inputFile.empty())
        {
            double width = widthWithMargins / 3 * 2, height = 0;
            const auto mountain = getImage(mako, testFilePath + inputFile, width, height);
            paragraphs[paraIndex]->addRun(ILayoutImageRun::create(mako, mountain, width, height));
        }
        
        for (uint32 paraCopyIndex = 3; paraCopyIndex < 8; ++paraCopyIndex)
        {
            paragraphs.append(createParagraph(7, ILayoutParagraph::eHAJustified, 1.2));
            paragraphs[++paraIndex]->addRun(
                ILayoutTextRun::create(paragraphCopy[paraCopyIndex], bodyFont.first, bodyFont.second, 14.0));
        }

        const auto david = getOpenTypeFont(mako, { "David Regular" });
        assert(david.first);
        const auto arial = getOpenTypeFont(mako, { "Arial" });
        assert(arial.first);

        // Right-to-left language test
        String david_header = L"סביבות מגורים - מהביוספירה ועד לנישה האקולוגית";
        String david_para = L".מרכזו של כדור-הארץ נמצא כמעט 6400 קילומטר מתחת לפני הקרקע. עד כה הצליח האדם להגיע רק לעומק של שמונה קילומטרים בקירוב. אולם כבר היום ידוע לנו כי קילומטרים ספורים מתחת לכפות רגלינו כבר עולה טמפרטורת הארץ למידה שאיננה מאפשרת קיום של חיים. קילומטרים ספורים מעל לראשנו האוויר נעשה דליל וקר, ולא מתאפשר בו קיום של חיים. בתווך מצויה הביוספירה (ביו=חיים, ספירה=עולם) ¬– שכבה דקה על פני כדור-הארץ שעובייה כקליפת תפוח-העץ ביחס לתפוח השלם, אשר בה מתקיים כל עושר החיים המוכר לנו. הביוספירה כוללת את מכלול החיים בים וביבשה, במחילות ובין הרגבים שמתחת לפני הקרקע ובשכבות האוויר הסמוכות לקרקע. הביוספירה היא סביבת החיים הגדולה ביותר המוכרת לנו";

        paragraphs.append(createParagraph(8, ILayoutParagraph::eHARight));
        paragraphs[++paraIndex]->addRun(ILayoutTextRun::create(StringToU8String(david_header), david.first, david.second, 16.0));
        paragraphs.append(createParagraph(6.5, ILayoutParagraph::eHARight));
        paragraphs[++paraIndex]->addRun(ILayoutTextRun::create(StringToU8String(david_para), arial.first, arial.second, 13.0));

        // Add to the page
        fixedPage->appendChild(layout->layout(paragraphs));

        // Write PDF
        const auto output = IPDFOutput::create(mako);
        output->setParameter("Producer", "Mako Layout Engine");
        output->writeAssembly(assembly, "MyFirstLayout(CPP).pdf");

        // Done
    }
    catch (IError& e)
    {
        const auto errorFormatString = getEDLErrorString(e.getErrorCode());
        std::wcerr << L"Exception: " << e.getErrorDescription(errorFormatString) << std::endl;
        return static_cast<int>(e.getErrorCode());
    }
    catch (std::exception& e)
    {
        std::wcerr << L"std::exception thrown: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}