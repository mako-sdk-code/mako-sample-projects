#
# -----------------------------------------------------------------------
# <copyright file="save_with_output_intent.py" company="Hybrid Software Helix Ltd">
#  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
# </copyright>
# <summary>
#  This example is provided on an "as is" basis and without warranty of any kind.
#  Hybrid Software Helix Ltd. does not warrant or make any representations
#  regarding the use or results of use of this example.
# </summary>
# -----------------------------------------------------------------------
#

import os
from ctypes import c_uint

from jawsmakoIF_python import *


def main():
    try:
        # Define ICC profiles and their URLs
        profiles = [
            ("Probev2_ICCv4.icc", "https://www.color.org/probeprofile.xalter"),
            ("GRACoL2006_Coated1v2.icc", "https://idealliance.org"),
            ("JapanColor2011Coated.icc", "https://www.xrite.com"),
        ]

        # Instantiate Mako
        jaws_mako = IJawsMako.create("")
        IJawsMako.enableAllFeatures(jaws_mako)

        factory = jaws_mako.getFactory()

        # Create a document and a page
        assembly = IDocumentAssembly.create(jaws_mako)
        document = IDocument.create(jaws_mako)
        assembly.appendDocument(document)

        page = IPage.create(jaws_mako)
        document.appendPage(page)

        fixed_page = IDOMFixedPage.create(factory, M2X(250), M2X(210))
        page.setContent(fixed_page)

        # Load font
        (font_handle, font_index) = jaws_mako.findFont("Arial")
        font = IDOMFontOpenType.fromRCObject(font_handle.toRCObject())

        # Create a dark blue color
        dark_blue = IDOMColor.createSolidRgb(factory, 0.0, 0.0, 0.5)

        # Create a layout with a frame
        layout = ILayout.create(jaws_mako)
        frame_rect = FRect(M2X(12), M2X(12), M2X(273), M2X(186))
        layout.addFrame(ILayoutFrame.create(frame_rect))

        header = ILayoutParagraph.create(
            ILayoutParagraph.eHALeft, P2X(6)
        )
        body = ILayoutParagraph.create(
            ILayoutParagraph.eHALeft, P2X(5)
        )

        paragraphs = CEDLVectILayoutParagraph()
        para_index = 0

        # Header text
        text = "This PDF has an Output Intent referencing this ICC profile:"
        run = ILayoutTextRun.create(text, font, font_index, P2X(12), dark_blue)
        paragraphs.append(header.clone())
        paragraphs[para_index].addRun(ILayoutRun.fromRCObject(run.toRCObject()))

        # Body text
        text = f"  ● {profiles[0][0]} (from {profiles[0][1]})"
        run = ILayoutTextRun.create(text, font, font_index, P2X(10))
        paragraphs.append(body.clone())
        para_index += 1
        paragraphs[para_index].addRun(ILayoutRun.fromRCObject(run.toRCObject()))

        # Insert image
        parrot = get_image(jaws_mako, os.path.join("TestFiles", "Parrot.png"))
        paragraphs.append(ILayoutParagraph.create())
        para_index += 1
        paragraphs[para_index].addRun(
            ILayoutImageRun.create(jaws_mako, parrot, 0, M2X(169))
        )

        # Layout content on the page
        fixed_page.appendChild(layout.layout(paragraphs))

        # Load ICC profile
        icc_profile_path = os.path.join("TestFiles", profiles[0][0])
        icc_profile = IDOMICCProfile.create(
            factory, IInputStream.createFromFile(factory, icc_profile_path)
        )

        # Create PDF/X-4 output
        output = IPDFOutput.create(jaws_mako)
        output.setPreset("PDF/X-4")

        subtype = "GTS_PDFX"
        registry_name = "http://www.color.org"
        output_condition = profiles[0][0]
        output_condition_identifier = profiles[0][0]
        info = "Output Intent test"

        # Create and add OutputIntent
        output_intent = IOutputIntent.create(
            jaws_mako,
            subtype,
            output_condition,
            output_condition_identifier,
            registry_name,
            info,
            icc_profile,
        )
        output.setOutputIntent(output_intent)

        # Write PDF
        output.writeAssembly(assembly, "OutputIntentExample.pdf")

        # Verify output intent count = 1
        test_doc = IPDFInput.create(jaws_mako).open("OutputIntentExample.pdf").getDocument()
        if test_doc.getOutputIntents().size() != 1:
            raise Exception("Output intent count not equal to 1")

        # Demonstrate multiple output intents
        output_intents = CEDLVectIOutputIntent()
        output_intents.append(output_intent)

        # GRACoL
        subtype = "GGS_DUMMY"
        registry_name = "https://idealliance.org"
        output_condition = profiles[1][0]
        output_condition_identifier = profiles[1][0]
        info = "The GRACoL 2006 Coated v2 from idealliance.org"
        output_intent = IOutputIntent.create(
            jaws_mako,
            subtype,
            output_condition,
            output_condition_identifier,
            registry_name,
            info,
            icc_profile,
        )
        output_intents.append(output_intent)

        # JapanColor
        subtype = "GGS_DUMMY"
        registry_name = "https://www.xrite.com"
        output_condition = profiles[2][0]
        output_condition_identifier = profiles[2][0]
        info = "The Japan Color 2011 profile from xrite.com"
        output_intent = IOutputIntent.create(
            jaws_mako,
            subtype,
            output_condition,
            output_condition_identifier,
            registry_name,
            info,
            icc_profile,
        )
        output_intents.append(output_intent)

        # Set and write multiple output intents
        output.setOutputIntents(output_intents)
        output.writeAssembly(assembly, "OutputIntentExampleThreeIntents.pdf")

        # Verify output intent count = 3
        test_doc = (
            IPDFInput.create(jaws_mako)
            .open("OutputIntentExampleThreeIntents.pdf")
            .getDocument()
        )
        if test_doc.getOutputIntents().size() != 3:
            raise Exception("Output intent count not equal to 3")

        print("✅ Output intents successfully created and verified.")

    except MakoException as e:
        print("MakoException:", e.m_msg)
    except Exception as e:
        print("Exception:", e)


# Points (1/72") to XPS units (1/96")
def P2X(value):
    return value / 72.0 * 96.0


# Millimetres to XPS units (1/96")
def M2X(value):
    return value / 25.4 * 96.0


# Load an image file and return as IDOMImage
def get_image(jaws_mako, image_path):
    if not os.path.exists(image_path):
        raise FileNotFoundError(f"Image file {image_path} not found.")

    ext = os.path.splitext(image_path)[1].lower()

    if ext == ".jpg":
        return IDOMJPEGImage.create(
            jaws_mako.getFactory(),
            IInputStream.createFromFile(jaws_mako.getFactory(), image_path),
        )
    elif ext == ".png":
        return IDOMPNGImage.create(
            jaws_mako.getFactory(),
            IInputStream.createFromFile(jaws_mako.getFactory(), image_path),
        )
    elif ext == ".tif":
        return IDOMTIFFImage.create(
            jaws_mako.getFactory(),
            IInputStream.createFromFile(jaws_mako.getFactory(), image_path),
        )
    else:
        raise Exception(f"Unsupported image type: {ext}")


if __name__ == "__main__":
    main()
