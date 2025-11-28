# ------------------------------------------------------------------------------
# <copyright file="ilayoutexample.py" company="Hybrid Software Helix Ltd">
#  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
# </copyright>
# <summary>
#  This example is provided on an "as is" basis and without warranty of any kind.
#  Hybrid Software Helix Ltd. does not warrant or make any representations
#  regarding the use or results of use of this example.
# </summary>
# ------------------------------------------------------------------------------

import os
import sys
from jawsmakoIF_python import *


def PT2XPS(value):
    return value / 72.0 * 96.0


def MM2XPS(value):
    return value / 25.4 * 96.0


def get_open_type_font(mako, fonts_to_try):
    font = None
    font_index = 0
    for name in fonts_to_try:
        try:
            font, font_index = mako.findFont(name)
            break
        except MakoException:
            font, font_index = mako.findFont("Arial")
    if font.getFontType() == IDOMFont.eFontTypeOpenType:
        return IDOMFontOpenType.fromRCObject(font), font_index
    return None, 0


def draw_frame(mako, frame_bounds):
    margin = 4
    border = FRect(frame_bounds.x - margin, frame_bounds.y - margin,
                   frame_bounds.dX + margin * 2, frame_bounds.dY + margin * 2)
    brush = IDOMSolidColorBrush.createSolidRgb(mako, 0.8, 0.8, 0.8)
    path = IDOMPathNode.createStroked(mako, IDOMPathGeometry.create(mako, border), brush)
    path.setStrokeThickness(1)
    return path


def add_frame(mako, layout, fixed_page, rect, draw_outline=False):
    layout.addFrame(ILayoutFrame.create(rect))
    if draw_outline:
        fixed_page.appendChild(draw_frame(mako, rect))


def create_paragraph(space_after=0.0, justification=ILayoutParagraph.eHALeft,
                     leading=1.0, space_before=0.0):
    para = ILayoutParagraph.create(justification)
    if space_after > 0.0:
        para.setSpacing(space_after)
    if space_before > 0.0:
        para.setSpacing(space_before, True)
    para.setLeading(leading)
    return para


def get_image(factory, image_file, width, height):
    if not os.path.exists(image_file):
        raise Exception(f"Image file {image_file} not found.")
    ext = os.path.splitext(image_file)[1].lower()
    if ext == ".jpg":
        image = IDOMJPEGImage.create(factory, IInputStream.createFromFile(factory, image_file))
    elif ext == ".png":
        image = IDOMPNGImage.create(factory, IInputStream.createFromFile(factory, image_file))
    elif ext == ".tif":
        image = IDOMTIFFImage.create(factory, IInputStream.createFromFile(factory, image_file))
    else:
        raise Exception(f"Unsupported image type {ext}")

    frame = image.getImageFrame(factory)
    img_w, img_h = frame.getWidth(), frame.getHeight()
    aspect = img_w / img_h

    if width == 0.0 and height == 0.0:
        return image, img_w, img_h
    if height == 0.0:
        height = width / aspect
    elif width == 0.0:
        width = height * aspect

    return image, width, height


def main():
    test_filepath = "TestFiles/"
    input_file = sys.argv[1] if len(sys.argv) > 1 else ""

    try:
        mako = IJawsMako.create("")
        IJawsMako.enableAllFeatures(mako)
        factory = mako.getFactory()

        assembly = IDocumentAssembly.create(mako)
        document = IDocument.create(mako)
        assembly.appendDocument(document)
        page = IPage.create(mako)
        document.appendPage(page)
        fixed_page = IDOMFixedPage.create(factory)
        page.setContent(fixed_page)

        layout = ILayout.create(mako)

        draw_border = False
        margin = MM2XPS(12)
        width_with_margins = fixed_page.getWidth() - margin * 2

        add_frame(mako, layout, fixed_page, FRect(margin, margin, width_with_margins, MM2XPS(40)), draw_border)
        add_frame(mako, layout, fixed_page, FRect(margin, margin + MM2XPS(40), width_with_margins / 3 - MM2XPS(2), MM2XPS(83)), draw_border)
        add_frame(mako, layout, fixed_page, FRect(margin + width_with_margins / 3, margin + MM2XPS(40), width_with_margins / 3 * 2, MM2XPS(83)), draw_border)
        add_frame(mako, layout, fixed_page, FRect(margin, margin + MM2XPS(125), width_with_margins, MM2XPS(155)), draw_border)

        title_font = get_open_type_font(mako, ["Arial Black"])
        body_font = get_open_type_font(mako, ["DejaVu Sans Book", "Gill Sans", "Arial"])
        body_bold = get_open_type_font(mako, ["DejaVu Sans Book Bold", "Gill Sans Bold", "Arial Bold"])
        dark_blue = IDOMColor.createSolidRgb(factory, 0.0, 0.0, 0.5)

        # Prepare text paragraphs
        paragraph_text = [
            "Travel Blog",
            "\nThe beauty of where I found myself inspired me to write this.",
            "As the sun rose over the horizon, casting a warm golden hue across the landscape, a breathtaking scene unfolded before the onlooker's eyes. Standing at the water's edge, one's gaze extended out over a pristine lake that shimmered like a sheet of glass, reflecting the majestic beauty that surrounded it. The lake seemed to hold its breath, mirroring with utmost precision the awe-inspiring sight that lay just beyond its tranquil surface.",
            "Stretching magnificently into the distance, a range of rocky mountains dominated the backdrop. Each peak soared towards the heavens, their rugged surfaces etched by the passage of time and the forces of nature. The mountains stood resolute, a testament to the immense power and grandeur of the natural world. Their colors shifted subtly, painted with a breathtaking array of earthy tones \u2014 from deep siennas and ochres to soft grays and greens \u2014 all framed by the azure expanse of the sky.",
            "The sky itself seemed to be a canvas of its own, an ever-changing masterpiece of color and light. Towering cumulus clouds danced gracefully, casting dramatic shadows that gently caressed the mountains' slopes. The fluffy white clouds looked as though they were soft pillows, inviting the weary soul to rest upon their tender embrace. The sky stretched boundlessly, seeming to touch the very edges of the earth, a reminder of the vastness of the universe and the limitless possibilities that lay beyond the human imagination.",
            "As a gentle breeze whispered through the air, ripples formed across the surface of the lake, momentarily distorting the mirror-like reflection. The tiny waves moved in rhythmic harmony, lending an animated quality to the otherwise still waters. With each passing gust, the mountains appeared to ripple across the lake's surface, as though a magnificent magic spell had been cast upon the land.",
            "In the distance, a lone boat glided silently across the lake, its presence adding a touch of serenity to the already tranquil scene. The boat's wake created a delicate trail on the water, a fleeting mark of human existence in the midst of the grandeur of nature. It served as a reminder of the delicate balance between mankind and the Earth, and how nature's beauty can be both admired and preserved.",
            "Birds soared overhead, their graceful silhouettes weaving intricate patterns against the sky. They navigated the currents with effortless grace, adding life to the enchanting tableau. The soft cries of the birds mixed harmoniously with the gentle lapping of the lake's water against the shore, creating a soothing symphony that echoed through the air."
        ]

        paragraphs = CEDLVectILayoutParagraph()
        idx = 0

        # Title
        paragraphs.append(ILayoutParagraph.create(ILayoutParagraph.eHACenter))
        run = ILayoutTextRun.create(paragraph_text[0], title_font[0], title_font[1], PT2XPS(60))
        run.setColor(dark_blue)
        paragraphs[idx].addRun(ILayoutRun.fromRCObject(run.toRCObject()))

        # Intro
        paragraphs.append(create_paragraph(9, ILayoutParagraph.eHAJustified, 1.3))
        idx += 1
        paragraphs[idx].addRun(ILayoutTextRun.create(paragraph_text[1], body_bold[0], body_bold[1], 14.0))

        # Sidebar
        paragraphs.append(create_paragraph(7, ILayoutParagraph.eHAJustified, 1.2))
        idx += 1
        paragraphs[idx].addRun(ILayoutTextRun.create(paragraph_text[2], body_font[0], body_font[1], 14.0))

        # Image frame
        if input_file:
            width = width_with_margins / 3 * 2
            height = 0
            image, width, height = get_image(factory, os.path.join(test_filepath, input_file), width, height)
            paragraphs[idx].addRun(ILayoutImageRun.create(mako, image, width, height))

        # Body paragraphs
        for i in range(3, len(paragraph_text)):
            paragraphs.append(create_paragraph(7, ILayoutParagraph.eHAJustified, 1.2))
            idx += 1
            paragraphs[idx].addRun(ILayoutTextRun.create(paragraph_text[i], body_font[0], body_font[1], 14.0))

        # Right-to-left language test
        david = get_open_type_font(mako, ["DavidRegular"])
        arial = get_open_type_font(mako, ["Arial"])
        david_header = "סביבות מגורים - מהביוספירה ועד לנישה האקולוגית"
        david_para = "מרכזו של כדור-הארץ נמצא כמעט 6400 קילומטר מתחת לפני הקרקע. עד כה הצליח האדם להגיע רק לעומק של שמונה קילומטרים בקירוב. אולם כבר היום ידוע לנו כי קילומטרים ספורים מתחת לכפות רגלינו כבר עולה טמפרטורת הארץ למידה שאיננה מאפשרת קיום של חיים. קילומטרים ספורים מעל לראשנו האוויר נעשה דליל וקר, ולא מתאפשר בו קיום של חיים. בתווך מצויה הביוספירה (ביו=חיים, ספירה=עולם) ¬– שכבה דקה על פני כדור-הארץ שעובייה כקליפת תפוח-העץ ביחס לתפוח השלם, אשר בה מתקיים כל עושר החיים המוכר לנו. הביוספירה כוללת את מכלול החיים בים וביבשה, במחילות ובין הרגבים שמתחת לפני הקרקע ובשכבות האוויר הסמוכות לקרקע. הביוספירה היא סביבת החיים הגדולה ביותר המוכרת לנו."

        paragraphs.append(create_paragraph(8, ILayoutParagraph.eHARight))
        idx += 1
        paragraphs[idx].addRun(ILayoutTextRun.create(david_header, david[0], david[1], 16.0))
        paragraphs.append(create_paragraph(6.5, ILayoutParagraph.eHARight))
        idx += 1
        paragraphs[idx].addRun(ILayoutTextRun.create(david_para, arial[0], arial[1], 13.0))

        # Add layout to page
        fixed_page.appendChild(layout.layout(paragraphs))

        output = IPDFOutput.create(mako)
        output.setParameter("maxAccumulatedPages", "1")
        output.setParameter("Producer", "Mako Layout Engine")
        output.writeAssembly(assembly, "MyFirstLayout(Python).pdf")

        print("Layout PDF written successfully!")

    except MakoException as e:
        print(f"MakoException: {e.m_msg}")
    except Exception as e:
        print(f"Exception thrown: {e}")


if __name__ == "__main__":
    main()
