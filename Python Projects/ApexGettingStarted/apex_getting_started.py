# -----------------------------------------------------------------------
# <copyright file="ApexGettingStarted.py" company="Hybrid Software Helix Ltd">
#  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
# </copyright>
# <summary>
#  This example is provided on an "as is" basis and without warranty of any kind.
#  Hybrid Software Helix Ltd. does not warrant or make any representations
#  regarding the use or results of use of this example.
# </summary>
# -----------------------------------------------------------------------

import sys
import os
from pathlib import Path
from jawsmakoIF_python import *

TEST_FILES_PATH = str(Path(__file__).resolve().parents[2] / "TestFiles") + os.sep

def main():
    try:
        # Create our JawsMako instance and enable all features
        jaws_mako = IJawsMako.create("")
        IJawsMako.enableAllFeatures(jaws_mako)

        # Open the document assembly, document, first page, and get its content
        input_pdf = IInput.create(jaws_mako, eFFPDF)
        assembly = input_pdf.open(TEST_FILES_PATH + "Cheshire Cat.pdf")
        document = assembly.getDocument()
        page = document.getPage(0)
        crop_box = page.getCropBox()
        content = page.getContent()

        # Create an Apex renderer
        renderer = IApexRenderer.create(jaws_mako)

        # Set up the image render spec
        image_spec = CImageRenderSpec()

        # 300 dpi rendering — calculate pixel dimensions
        image_spec.width = int(page.getWidth() / 96.0 * 300.0)
        image_spec.height = int(page.getHeight() / 96.0 * 300.0)

        # Specify source rectangle and process color space
        image_spec.sourceRect = crop_box
        image_spec.processSpace = IDOMColorSpaceDeviceRGB.create(jaws_mako.getFactory())

        # Render page content to image
        renderer.render(content, image_spec)

        # Fetch result image and encode to PNG
        image = image_spec.result
        output_stream = IOutputStream.createToFile(jaws_mako.getFactory(), TEST_FILES_PATH + "Cheshire Cat.png")
        IDOMPNGImage.encode(jaws_mako, image, output_stream)

        print("Rendered page saved as Cheshire Cat.png")
        return 0

    except MakoException as e:
        print("MakoException thrown:", e.m_msg)
        return 1

    except Exception as e:
        print("std::exception thrown:", str(e))
        return 1


if __name__ == "__main__":
    sys.exit(main())
