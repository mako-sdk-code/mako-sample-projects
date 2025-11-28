# -----------------------------------------------------------------------
# <copyright file="text_transform_example.py" company="Hybrid Software Helix Ltd">
#  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
# </copyright>
# <summary>
#  This example is provided on an "as is" basis and without warranty of any kind.
#  Hybrid Software Helix Ltd. does not warrant or make any representations
#  regarding the use or results of use of this example.
# </summary>
# -----------------------------------------------------------------------

import sys
import math
from jawsmakoIF_python import *;

class TextTransformImplementation( ICustomTransform.IImplementation):
    """
    Custom transform implementation to adjust black text ink values.
    """

    def __init__(self, jaws_mako, text_ink_value):
        super().__init__()
        self.m_mako = jaws_mako
        self.m_factory = jaws_mako.getFactory()
        self.m_textInkValue = text_ink_value

    def transformGlyphs(self, genericImplementation, glyphs, changed, state):
        try:
            fill = glyphs.getFill()
            if fill.getBrushType() != IDOMBrush.eSolidColor:
                return genericImplementation.transformGlyphs(None, glyphs, changed, state)

            colorBrush =  IDOMSolidColorBrush.fromRCObject(fill)
            color = colorBrush.getColor()

            if (color.getColorSpace().equals( IDOMColorSpaceDeviceCMYK.create(self.m_factory))
                and abs(color.getComponentValue(3) - 1.0) < 0.0001):
                newBrush =  IDOMSolidColorBrush.createSolidCmyk(
                    self.m_factory, 0.0, 0.0, 0.0, self.m_textInkValue)
                glyphs.setFill(newBrush)
                changed = True
                return glyphs

        except  MakoException as e:
            error_str =  getEDLErrorString(e.m_errorCode)
            print(f"Exception thrown: {e.m_msg}", file=sys.stderr)
        except Exception as e:
            print(f"std::exception thrown: {e}", file=sys.stderr)

        return genericImplementation.transformGlyphs(None, glyphs, changed, state)


def main():
    if len(sys.argv) != 4:
        print(f"Usage: {sys.argv[0]} <input> <output> <inkValue>", file=sys.stderr)
        sys.exit(1)

    test_file_path = "TestFiles/"
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    ink_value = float(sys.argv[3])

    try:
        # Instantiate Mako
        jaws_mako =  IJawsMako.create("")
        IJawsMako.enableAllFeatures(jaws_mako)

        # Create our custom transform
        implementation = TextTransformImplementation(jaws_mako, ink_value)
        text_modifier =  ICustomTransform.create(
            jaws_mako,
            implementation,
        )

        # Open the input PDF
        assembly =  IInput.create(jaws_mako, eFFPDF).open(test_file_path + input_file)
        document = assembly.getDocument()

        page_count = document.getNumPages()
        print(f"Processing {page_count} pages...")

        for i in range(page_count):
            page = document.getPage(i)
            changed = False
            text_modifier.transformPage(page, changed)
            page.release()

        print("Writing output file ...")
        pdf_output =  IPDFOutput.create(jaws_mako)
        pdf_output.writeAssembly(assembly, output_file)

    except MakoException as e:
        error_str =  getEDLErrorString(e.m_errorCode)
        print(f"Exception thrown: {e.m_msg}", file=sys.stderr)
        sys.exit(int(e.m_errorCode))
    except Exception as e:
        print(f"std::exception thrown: {e}", file=sys.stderr)
        sys.exit(1)

    sys.exit(0)


if __name__ == "__main__":
    main()
