/* -----------------------------------------------------------------------
 * <copyright file="Main.java" company="Hybrid Software Helix Ltd">
 *  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Hybrid Software Helix Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 * </summary>
 * -----------------------------------------------------------------------
 */

import java.util.ArrayList;
import java.util.Objects;

import com.globalgraphics.JawsMako.jawsmakoIF.*;

public class LayoutExample
{
    public static void main(String[] args)
    {
        var testFilePath = "TestFiles/";
        var inputFile = args.length > 0 ? args[0] : "";

        try
        {
            var mako = IJawsMako.create();
            IJawsMako.enableAllFeatures(mako);
            IEDLClassFactory factory = mako.getFactory ();

            // Get a page ready to accept some DOM
            var assembly = IDocumentAssembly.create(mako);
            var document = IDocument.create(mako);
            assembly.appendDocument(document);
            var page = IPage.create(mako);
            document.appendPage(page);
            var fixedPage = IDOMFixedPage.create(factory);
            page.setContent(fixedPage);

            // Create a layout
            var layout = ILayout.create(mako);

            // Add some frames to hold content
            boolean drawBorder = false;
            var margin = MM2XPS(12);
            var widthWithMargins = fixedPage.getWidth() - margin * 2;
            AddFrame(mako, layout, fixedPage, new FRect(margin, margin, widthWithMargins, MM2XPS(40)), drawBorder);                                                // Banner
            AddFrame(mako, layout, fixedPage, new FRect(margin, margin + MM2XPS(40), widthWithMargins / 3 - MM2XPS(2), MM2XPS(83)), drawBorder);                   // Sidebar with 2mm right margin
            AddFrame(mako, layout, fixedPage, new FRect(margin + widthWithMargins / 3, margin + MM2XPS(40), widthWithMargins / 3 * 2, MM2XPS(83)), drawBorder);    // Pic
            AddFrame(mako, layout, fixedPage, new FRect(margin, margin + MM2XPS(125), widthWithMargins, MM2XPS(155)), drawBorder);

            // Get a font
            var titleFont = new GetOpenTypeFont(mako, new ArrayList<>() {{ add("Arial Black"); }});
            var bodyFont = new GetOpenTypeFont(mako, new ArrayList<>() {{ add("DejaVu Sans Book"); add("Gill Sans"); add("Arial"); }});
            var bodyBold = new GetOpenTypeFont(mako, new ArrayList<>() {{ add("DejaVu Sans Book Bold"); add("Gill Sans Bold"); add("Arial Bold"); }});

            // Create a colour
            var darkBlue = IDOMColor.createSolidRgb(mako.getFactory(), 0.0f, 0.0f, 0.5f);

            var paragraphCopy = new ArrayList<String>();
            paragraphCopy.add("Travel Blog");
            paragraphCopy.add("\nThe beauty of where I found myself inspired me to write this.");
            paragraphCopy.add("As the sun rose over the horizon, casting a warm golden hue across the landscape, a breathtaking scene unfolded before the onlooker's eyes. Standing at the water's edge, one's gaze extended out over a pristine lake that shimmered like a sheet of glass, reflecting the majestic beauty that surrounded it. The lake seemed to hold its breath, mirroring with utmost precision the awe-inspiring sight that lay just beyond its tranquil surface.");
            paragraphCopy.add("Stretching magnificently into the distance, a range of rocky mountains dominated the backdrop. Each peak soared towards the heavens, their rugged surfaces etched by the passage of time and the forces of nature. The mountains stood resolute, a testament to the immense power and grandeur of the natural world. Their colors shifted subtly, painted with a breathtaking array of earthy tones \u2014 from deep siennas and ochres to soft grays and greens \u2014 all framed by the azure expanse of the sky.");
            paragraphCopy.add("The sky itself seemed to be a canvas of its own, an ever-changing masterpiece of color and light. Towering cumulus clouds danced gracefully, casting dramatic shadows that gently caressed the mountains' slopes. The fluffy white clouds looked as though they were soft pillows, inviting the weary soul to rest upon their tender embrace. The sky stretched boundlessly, seeming to touch the very edges of the earth, a reminder of the vastness of the universe and the limitless possibilities that lay beyond the human imagination.");
            paragraphCopy.add("As a gentle breeze whispered through the air, ripples formed across the surface of the lake, momentarily distorting the mirror-like reflection. The tiny waves moved in rhythmic harmony, lending an animated quality to the otherwise still waters. With each passing gust, the mountains appeared to ripple across the lake's surface, as though a magnificent magic spell had been cast upon the land.");
            paragraphCopy.add("In the distance, a lone boat glided silently across the lake, its presence adding a touch of serenity to the already tranquil scene. The boat's wake created a delicate trail on the water, a fleeting mark of human existence in the midst of the grandeur of nature. It served as a reminder of the delicate balance between mankind and the Earth, and how nature's beauty can be both admired and preserved.");
            paragraphCopy.add("Birds soared overhead, their graceful silhouettes weaving intricate patterns against the sky. They navigated the currents with effortless grace, adding life to the enchanting tableau. The soft cries of the birds mixed harmoniously with the gentle lapping of the lake's water against the shore, creating a soothing symphony that echoed through the air.");
            paragraphCopy.add("");

            // A vector to point to each paragraph to be added to the frame(s)
            CEDLVectILayoutParagraph paragraphs = new CEDLVectILayoutParagraph();
            long paraIndex = 0;

            // Create paragraphs and add text runs to them

            // Title
            paragraphs.append(ILayoutParagraph.create(ILayoutParagraph.eHorizontalAlignment.eHACenter));
            var run = ILayoutTextRun.create(paragraphCopy.get(0), titleFont.font, titleFont.index, PT2XPS(60));
            run.setColor(darkBlue);
            paragraphs.getitem(paraIndex).addRun(run);

            // Intro
            paragraphs.append(CreateParagraph(9, ILayoutParagraph.eHorizontalAlignment.eHAJustified, 1.3, 0.0));
            paragraphs.getitem(++paraIndex).addRun(
                    ILayoutTextRun.create(paragraphCopy.get(1), bodyBold.font, bodyBold.index, 14.0));

            // Sidebar
            paragraphs.append(CreateParagraph(7, ILayoutParagraph.eHorizontalAlignment.eHAJustified, 1.2, 0.0));
            paragraphs.getitem(++paraIndex).addRun(
                    ILayoutTextRun.create(paragraphCopy.get(2), bodyFont.font, bodyFont.index, 14.0));

            // Break to next frame
            paragraphs.append(ILayoutParagraph.create());
            paragraphs.getitem(++paraIndex).addRun(ILayoutTextRun.create("\n", bodyFont.font, bodyFont.index, 4));

            // Picture
            if (!Objects.equals(inputFile, ""))
            {
                double width = widthWithMargins / 3 * 2, height = 0;
                var mountain = new GetImage(mako, testFilePath + inputFile, width, height);
                paragraphs.getitem(paraIndex).addRun(ILayoutImageRun.create(mako, mountain.image, mountain.width, mountain.height));
            }

            for (int paraCopyIndex = 3; paraCopyIndex < 8; paraCopyIndex++)
            {
                paragraphs.append(CreateParagraph(7.0, ILayoutParagraph.eHorizontalAlignment.eHAJustified, 1.2, 0.0));
                paragraphs.getitem(++paraIndex).addRun(
                        ILayoutTextRun.create(paragraphCopy.get(paraCopyIndex), bodyFont.font, bodyFont.index, 14.0));
            }
            var david = new GetOpenTypeFont(mako, new ArrayList<>() {{ add( "DavidRegular"); }});
            var arial = new GetOpenTypeFont(mako, new ArrayList<>() {{ add( "Arial"); }});

            // Right-to-left language test
            String heb_header = "סביבות מגורים - מהביוספירה ועד לנישה האקולוגית";
            String heb_para =
                    "מרכזו של כדור-הארץ נמצא כמעט 6400 קילומטר מתחת לפני הקרקע. עד כה הצליח האדם להגיע רק לעומק של שמונה קילומטרים בקירוב. אולם כבר היום ידוע לנו כי קילומטרים ספורים מתחת לכפות רגלינו כבר עולה טמפרטורת הארץ למידה שאיננה מאפשרת קיום של חיים. קילומטרים ספורים מעל לראשנו האוויר נעשה דליל וקר, ולא מתאפשר בו קיום של חיים. בתווך מצויה הביוספירה (ביו=חיים, ספירה=עולם) ¬– שכבה דקה על פני כדור-הארץ שעובייה כקליפת תפוח-העץ ביחס לתפוח השלם, אשר בה מתקיים כל עושר החיים המוכר לנו. הביוספירה כוללת את מכלול החיים בים וביבשה, במחילות ובין הרגבים שמתחת לפני הקרקע ובשכבות האוויר הסמוכות לקרקע. הביוספירה היא סביבת החיים הגדולה ביותר המוכרת לנו.";

            paragraphs.append(CreateParagraph(8, ILayoutParagraph.eHorizontalAlignment.eHARight, 1.0, 0.0));
            paragraphs.getitem(++paraIndex).addRun(ILayoutTextRun.create(heb_header, david.font, david.index, 16.0));
            paragraphs.append(CreateParagraph(6.5, ILayoutParagraph.eHorizontalAlignment.eHARight, 1.0, 0.0));
            paragraphs.getitem(++paraIndex).addRun(ILayoutTextRun.create(heb_para, arial.font, arial.index, 13.0));

            // Add to the page
            fixedPage.appendChild(layout.layout(paragraphs));

            // Write PDF
            var output = IPDFOutput.create(mako);
            output.setParameter("Producer", "Mako Layout Engine");
            output.writeAssembly(assembly, "MyFirstLayout(Java).pdf");

            // Done

        } catch (Exception e) {
            e.printStackTrace();
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

    // Draw a path using a solid color brush with a thickness of 1
    private static IDOMPathNode DrawFrame(IEDLClassFactory factory, FRect frameBounds)
    {
        double margin = 4;
        var border = new FRect(frameBounds.getX() - margin, frameBounds.getY() - margin,
                frameBounds.getDX() + margin * 2,frameBounds.getDY() + margin * 2);
        var solidBrush = IDOMSolidColorBrush.createSolidRgb(factory, 0.8f, 0.8f, 0.8f);
        var path = IDOMPathNode.createStroked(factory, IDOMPathGeometry.create(factory, border), solidBrush);
        path.setStrokeThickness(1);
        return path;
    }

    // Add a layout, draw an outline if requested
    private static void AddFrame(IJawsMako mako, ILayout layout, IDOMFixedPage fixedPage, FRect positionAndSize, boolean drawOutline)
    {
        layout.addFrame(ILayoutFrame.create(positionAndSize));

        // Draw a box where the frame is going to be placed
        if (drawOutline)
            fixedPage.appendChild(DrawFrame(mako.getFactory(), positionAndSize));
    }

    // Create paragraph
    private static ILayoutParagraph CreateParagraph(
            double spaceAfter,
            ILayoutParagraph.eHorizontalAlignment justification,
            double leading,
            double spaceBefore)
    {
        var templateParagraph = ILayoutParagraph.create(justification);
        if (spaceAfter > 0.0)
            templateParagraph.setSpacing(spaceAfter);
        if (spaceBefore > 0.0)
            templateParagraph.setSpacing(spaceBefore, true);
        templateParagraph.setLeading(leading);
        return templateParagraph;
    }
}