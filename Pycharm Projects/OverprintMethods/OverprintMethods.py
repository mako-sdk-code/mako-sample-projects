#
# -----------------------------------------------------------------------
# <copyright file="OverprintMethods.py" company="Hybrid Software Helix Ltd">
#  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
# </copyright>
# <summary>
#  This example is provided on an "as is" basis and without warranty of any kind.
#  Hybrid Software Helix Ltd. does not warrant or make any representations
#  regarding the use or results of use of this example.
# </summary>
# -----------------------------------------------------------------------
#

import sys
from jawsmakoIF_python import *

def main():
    test_file_path = "TestFiles/"

    try:
        # Create the Mako instance
        mako = IJawsMako.create("")
        IJawsMako.enableAllFeatures(mako)

        # Open the input PDF
        file_input = IInput.create(mako, eFFPDF)
        assembly = file_input.open(test_file_path + "CMYK_Circles 1.pdf")
        document = assembly.getDocument()

        # Apply overprint to all path nodes in all pages
        for page_index in range(document.getNumPages()):
            page = document.getPage(page_index)
            fixed_page = page.edit()

            path_nodes = fixed_page.findChildrenOfType(eDOMPathNode, True)

            for i in range(path_nodes.size()):
                path = IDOMPathNode.fromRCObject(path_nodes.getitem(i))
                if path:
                    # Apply overprint fill using the new API
                    path.setFillOverprints(True)
                    # If needed, apply overprint stroke
                    # path.setStrokeOverprints(True)
                    path.setOverprintMode(True)

        # Write to output PDF
        pdf_output = IPDFOutput.create(mako)
        pdf_output.writeAssembly(assembly, "test.pdf")

    except MakoException as ex:
        print("MakoException thrown:", ex.m_msg)
        return int(ex.m_errorCode)
    except Exception as e:
        print("Exception thrown:", e)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
