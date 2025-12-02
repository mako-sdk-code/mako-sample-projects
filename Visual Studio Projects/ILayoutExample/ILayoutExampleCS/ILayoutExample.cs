/* --------------------------------------------------------------------------------
*  <copyright file="ILayoutExample.cs" company="Hybrid Software Helix Ltd">
*    Copyright (c) 2025 Hybrid Software Helix Ltd. All rights reserved.
*  </copyright>
*  <summary>
*    This example is provided on an "as is" basis and without warranty of any kind.
*    Hybrid Software Helix Ltd. does not warrant or make any representations
*    regarding the use or results of use of this example.
*  </summary>
* ---------------------------------------------------------------------------------
*/

using JawsMako;
using System;
using static JawsMako.jawsmakoIF_csharp;
using static System.Runtime.InteropServices.JavaScript.JSType;

namespace LayoutExampleCS;

class ILayoutExample
{
    static void Main(string[] args)
    {
        var testFilepath = @"..\..\..\..\TestFiles\";
        string inputFile = args.Length > 0 ? args[0] : "";

        try
        {
            var mako = IJawsMako.create();
            IJawsMako.enableAllFeatures(mako);

            // Get a page ready to accept some DOM
            using var assembly = IDocumentAssembly.create(mako);
            using var document = IDocument.create(mako);
            assembly.appendDocument(document);
            using var page = IPage.create(mako);
            document.appendPage(page);
            var fixedPage = IDOMFixedPage.create(mako);
            page.setContent(fixedPage);

            // Create a layout
            var layout = ILayout.create(mako);

            // Add some frames to hold content
            bool drawBorder = false;
            var margin = MM2XPS(12);
            var widthWithMargins = fixedPage.getWidth() - margin * 2;
            AddFrame(ref mako, ref layout, ref fixedPage, new FRect(margin, margin, widthWithMargins, MM2XPS(40)), drawBorder);                                                // Banner
            AddFrame(ref mako, ref layout, ref fixedPage, new FRect(margin, margin + MM2XPS(40), widthWithMargins / 3 - MM2XPS(2), MM2XPS(83)), drawBorder);                   // Sidebar with 2mm right margin
            AddFrame(ref mako, ref layout, ref fixedPage, new FRect(margin + widthWithMargins / 3, margin + MM2XPS(40), widthWithMargins / 3 * 2, MM2XPS(83)), drawBorder);    // Pic
            AddFrame(ref mako, ref layout, ref fixedPage, new FRect(margin, margin + MM2XPS(125), widthWithMargins, MM2XPS(155)), drawBorder);                                 // Body

            // Get a font
            var titleFont = GetOpenTypeFont(mako, new List<string>(new[] { "Arial Black" }));
            var bodyFont = GetOpenTypeFont(mako, new List<string>(new[] { "DejaVu Sans Book", "Gill Sans", "Arial" }));
            var bodyBold = GetOpenTypeFont(mako, new List<string>(new[] { "DejaVu Sans Book Bold", "Gill Sans Bold", "Arial Bold" }));

            // Create a colour
            var darkBlue = IDOMColor.createSolidRgb(mako, 0.0f, 0.0f, 0.5f);

            CEDLVectString paragraphCopy = new CEDLVectString();

            paragraphCopy.append("Travel Blog");
            paragraphCopy.append("\nThe beauty of where I found myself inspired me to write this.");
            paragraphCopy.append("As the sun rose over the horizon, casting a warm golden hue across the landscape, a breathtaking scene unfolded before the onlooker's eyes. Standing at the water's edge, one's gaze extended out over a pristine lake that shimmered like a sheet of glass, reflecting the majestic beauty that surrounded it. The lake seemed to hold its breath, mirroring with utmost precision the awe-inspiring sight that lay just beyond its tranquil surface.");
            paragraphCopy.append("Stretching magnificently into the distance, a range of rocky mountains dominated the backdrop. Each peak soared towards the heavens, their rugged surfaces etched by the passage of time and the forces of nature. The mountains stood resolute, a testament to the immense power and grandeur of the natural world. Their colors shifted subtly, painted with a breathtaking array of earthy tones \u2014 from deep siennas and ochres to soft grays and greens \u2014 all framed by the azure expanse of the sky.");
            paragraphCopy.append("The sky itself seemed to be a canvas of its own, an ever-changing masterpiece of color and light. Towering cumulus clouds danced gracefully, casting dramatic shadows that gently caressed the mountains' slopes. The fluffy white clouds looked as though they were soft pillows, inviting the weary soul to rest upon their tender embrace. The sky stretched boundlessly, seeming to touch the very edges of the earth, a reminder of the vastness of the universe and the limitless possibilities that lay beyond the human imagination.");
            paragraphCopy.append("As a gentle breeze whispered through the air, ripples formed across the surface of the lake, momentarily distorting the mirror-like reflection. The tiny waves moved in rhythmic harmony, lending an animated quality to the otherwise still waters. With each passing gust, the mountains appeared to ripple across the lake's surface, as though a magnificent magic spell had been cast upon the land.");
            paragraphCopy.append("In the distance, a lone boat glided silently across the lake, its presence adding a touch of serenity to the already tranquil scene. The boat's wake created a delicate trail on the water, a fleeting mark of human existence in the midst of the grandeur of nature. It served as a reminder of the delicate balance between mankind and the Earth, and how nature's beauty can be both admired and preserved.");
            paragraphCopy.append("Birds soared overhead, their graceful silhouettes weaving intricate patterns against the sky. They navigated the currents with effortless grace, adding life to the enchanting tableau. The soft cries of the birds mixed harmoniously with the gentle lapping of the lake's water against the shore, creating a soothing symphony that echoed through the air.");
            paragraphCopy.append("");

            // A vector to point to each paragraph to be added to the frame(s)
            CEDLVectILayoutParagraph paragraphs = new CEDLVectILayoutParagraph();
            uint paraIndex = 0;

            // Create paragraphs and add text runs to them

            // Title
            paragraphs.append(ILayoutParagraph.create(ILayoutParagraph.eHorizontalAlignment.eHACenter));
            var run = ILayoutTextRun.create(paragraphCopy[0], titleFont.Item1, titleFont.Item2, PT2XPS(60));
            run.setColor(darkBlue);
            paragraphs[paraIndex].addRun(ILayoutRun.fromRCObject(run.toRCObject()));

            // Intro
            paragraphs.append(CreateParagraph(9, ILayoutParagraph.eHorizontalAlignment.eHAJustified, 1.3));
            paragraphs[++paraIndex].addRun(
                ILayoutTextRun.create(paragraphCopy[1], bodyBold.Item1, bodyBold.Item2, 14.0));

            // Side bar
            paragraphs.append(CreateParagraph(7, ILayoutParagraph.eHorizontalAlignment.eHAJustified, 1.2));
            paragraphs[++paraIndex].addRun(
                ILayoutTextRun.create(paragraphCopy[2], bodyFont.Item1, bodyFont.Item2, 14.0));

            // Break to next frame
            paragraphs.append(ILayoutParagraph.create());
            paragraphs[++paraIndex].addRun(ILayoutTextRun.create("\n", bodyFont.Item1, bodyFont.Item2, 4));

            // Picture
            if (inputFile != "")
            {
                double width = widthWithMargins / 3 * 2, height = 0;
                var mountain = GetImage(ref mako, testFilepath + inputFile, ref width, ref height);
                paragraphs[paraIndex].addRun(ILayoutImageRun.create(mako, mountain, width, height));
            }
            
            for (uint paraCopyIndex = 3; paraCopyIndex < 8; paraCopyIndex++)
            {
                paragraphs.append(CreateParagraph(7, ILayoutParagraph.eHorizontalAlignment.eHAJustified, 1.2));
                paragraphs[++paraIndex].addRun(
                    ILayoutTextRun.create(paragraphCopy[paraCopyIndex], bodyFont.Item1, bodyFont.Item2, 14.0));
            }

            var david = GetOpenTypeFont(mako, new List<string>(new[] { "DavidRegular" }));
            var arial = GetOpenTypeFont(mako, new List<string>(new[] { "Arial" }));

            // Right-to-left language test
            string david_header = "סביבות מגורים - מהביוספירה ועד לנישה האקולוגית";
            string david_para =
                "מרכזו של כדור-הארץ נמצא כמעט 6400 קילומטר מתחת לפני הקרקע. עד כה הצליח האדם להגיע רק לעומק של שמונה קילומטרים בקירוב. אולם כבר היום ידוע לנו כי קילומטרים ספורים מתחת לכפות רגלינו כבר עולה טמפרטורת הארץ למידה שאיננה מאפשרת קיום של חיים. קילומטרים ספורים מעל לראשנו האוויר נעשה דליל וקר, ולא מתאפשר בו קיום של חיים. בתווך מצויה הביוספירה (ביו=חיים, ספירה=עולם) ¬– שכבה דקה על פני כדור-הארץ שעובייה כקליפת תפוח-העץ ביחס לתפוח השלם, אשר בה מתקיים כל עושר החיים המוכר לנו. הביוספירה כוללת את מכלול החיים בים וביבשה, במחילות ובין הרגבים שמתחת לפני הקרקע ובשכבות האוויר הסמוכות לקרקע. הביוספירה היא סביבת החיים הגדולה ביותר המוכרת לנו.";

            paragraphs.append(CreateParagraph(8, ILayoutParagraph.eHorizontalAlignment.eHARight));
            paragraphs[++paraIndex].addRun(ILayoutTextRun.create(david_header, david.Item1, david.Item2, 16.0));
            paragraphs.append(CreateParagraph(6.5, ILayoutParagraph.eHorizontalAlignment.eHARight));
            paragraphs[++paraIndex].addRun(ILayoutTextRun.create(david_para, arial.Item1, arial.Item2, 13.0));

            // Add to the page
            fixedPage.appendChild(layout.layout(paragraphs));

            // Write PDF
            using var output = IPDFOutput.create(mako);
            output.setParameter("maxAccumulatedPages", "1");        // MAKOSUP-11222
            output.setParameter("Producer", "Mako Layout Engine");
            output.writeAssembly(assembly, "MyFirstLayout(CS).pdf");

            // Done
        }
        catch (MakoException e)
        {
            var errorFormatString = getEDLErrorString(e.m_errorCode);
            Console.WriteLine("Exception thrown: " + e.m_msg);
        }

        catch (Exception e)
        {
            Console.WriteLine($"Exception thrown: {e}");
        }
    }

    public static double PT2XPS(double value)
    {
        return value / 72.0 * 96.0;
    }

    public static double MM2XPS(double value)
    {
        return value / 25.4 * 96.0;
    }


    // Get font
    static Tuple<IDOMFontOpenType, uint> GetOpenTypeFont(IJawsMako mako, List<string> fontsToTry)
    {
        // Pick a font
        IDOMFont font = null!;
        uint fontIndex = 0; // In case the font is inside a TrueType collection
        foreach (var fontToTry in fontsToTry)
        {
            try
            {
                font = mako.findFont(fontToTry, out fontIndex);
                break;
            }
            catch (MakoException)
            {
                // Bad or missing font - default to Arial
                font = mako.findFont("Arial", out fontIndex);
            }
        }

        if (font!.getFontType() == IDOMFont.eFontType.eFontTypeOpenType)
            return new Tuple<IDOMFontOpenType, uint>(IDOMFontOpenType.fromRCObject(font), fontIndex);

        return new Tuple<IDOMFontOpenType, uint>(null!, 0);
    }

    // Draw a path using a solid color brush with a thickness of 1
    private static IDOMPathNode DrawFrame(IJawsMako mako, FRect frameBounds)
    {
        double margin = 4;
        using var border = new FRect(frameBounds.x - margin, frameBounds.y - margin, frameBounds.dX + margin * 2,
            frameBounds.dY + margin * 2);
        using var solidBrush = IDOMSolidColorBrush.createSolidRgb(mako, 0.8f, 0.8f, 0.8f);
        var path = IDOMPathNode.createStroked(mako, IDOMPathGeometry.create(mako, border), solidBrush);
        path.setStrokeThickness(1);
        return path;
    }

    // Add a layout, draw an outline if requested
    private static void AddFrame(ref IJawsMako mako, ref ILayout layout, ref IDOMFixedPage fixedPage, FRect positionAndSize, bool drawOutline = false)
    {
        layout.addFrame(ILayoutFrame.create(positionAndSize));

        // Draw a box where the frame is going to be placed
        if (drawOutline)
            fixedPage.appendChild(DrawFrame(mako, positionAndSize));
    }

    // Create paragraph
    private static ILayoutParagraph CreateParagraph(
        double spaceAfter = 0.0,
        ILayoutParagraph.eHorizontalAlignment justification = ILayoutParagraph.eHorizontalAlignment.eHALeft,
        double leading = 1.0,
        double spaceBefore = 0.0)
    {
        var templateParagraph = ILayoutParagraph.create(justification);
        if (spaceAfter > 0.0)
            templateParagraph.setSpacing(spaceAfter);
        if (spaceBefore > 0.0)
            templateParagraph.setSpacing(spaceBefore, true);
        templateParagraph.setLeading(leading);
        return templateParagraph;
    }

    // Get image and scale proportionately to specified width and/or height
    private static IDOMImage GetImage(ref IJawsMako mako, string imageFile, ref double width, ref double height)
    {

        var fileInfo = new FileInfo(imageFile);
        if (!fileInfo.Exists)
        {
            throw new Exception($"Image file {imageFile} not found.");
        }

        IDOMImage? image = null;

        if (fileInfo.Extension == ".jpg")
            image = IDOMJPEGImage.create(mako, IInputStream.createFromFile(mako, imageFile));

        if (fileInfo.Extension == ".png")
            image = IDOMPNGImage.create(mako, IInputStream.createFromFile(mako, imageFile));

        if (fileInfo.Extension == ".tif")
            image = IDOMTIFFImage.create(mako, IInputStream.createFromFile(mako, imageFile));

        if (image == null)
        {
            throw new Exception($"Image file {imageFile} could not be loaded.");
        }

        var frame = image.getImageFrame(mako);
        double imageWidth = frame.getWidth();
        double imageHeight = frame.getHeight();
        double aspectRatio = imageWidth / imageHeight;

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
}