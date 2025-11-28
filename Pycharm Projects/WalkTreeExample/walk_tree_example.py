# --------------------------------------------------------------------------------
#  <copyright file="walktree.py" company="Hybrid Software Helix Ltd">
#    Copyright (c) 2025 Hybrid Software Helix Ltd. All rights reserved.
#  </copyright>
#  <summary>
#    This example is provided on an "as is" basis and without warranty of any kind.
#    Hybrid Software Helix Ltd. does not warrant or make any representations
#    regarding the use or results of use of this example.
#  </summary>
# --------------------------------------------------------------------------------

import sys
from jawsmakoIF_python import *

class NodeTreeCallback(WalkTreeCallback):
    """Custom tree visitor callback class"""
    def visitNode(self, node):
        dump_node_info(node)
        return True


def dump_node_info(node):
    """Recursively indents and prints node type"""
    p = node
    while p.getParentNode() is not None:
        sys.stdout.write("  ")
        p = p.getParentNode()

    node_type = node.getNodeType()

    # Map the node type enum to readable names
    type_map = {
        eDOMFixedPageNode: "eDOMFixedPageNode",
        eDOMGroupNode: "eDOMGroupNode",
        eDOMCharPathGroupNode: "eDOMCharPathGroupNode",
        eDOMTransparencyGroupNode: "eDOMTransparencyGroupNode",
        eDOMGlyphsNode: "eDOMGlyphsNode",
        eDOMPathNode: "eDOMPathNode",
        eDOMFormNode: "eDOMFormNode",
        eDOMFormInstanceNode: "eDOMFormInstanceNode",
        eDOMContentRootNode: "eDOMContentRootNode",
        eDOMDocumentSequenceNode: "eDOMDocumentSequenceNode",
        eDOMDocumentNode: "eDOMDocumentNode",
        eDOMFixedDocumentNode: "eDOMFixedDocumentNode",
        eDOMPageNode: "eDOMPageNode",
        eDOMCanvasNode: "eDOMCanvasNode",
        eDOMGlyphNode: "eDOMGlyphNode",
        eDOMRefNode: "eDOMRefNode",
        eDOMJobTkContentNode: "eDOMJobTkContentNode",
        eDOMJobTkNodeNode: "eDOMJobTkNodeNode",
        eDOMJobTkValueNode: "eDOMJobTkValueNode",
        eDOMJobTkGenericNodeNode: "eDOMJobTkGenericNodeNode",
        eDOMJobTkGenericCharacterDataNode: "eDOMJobTkGenericCharacterDataNode",
        eDOMVisualRootNode: "eDOMVisualRootNode",
        eDOMOutlineNode: "eDOMOutlineNode",
        eDOMOutlineEntryNode: "eDOMOutlineEntryNode",
        eDOMAnnotationAppearanceNode: "eDOMAnnotationAppearanceNode",
        eDOMFragmentNode: "eDOMFragmentNode",
        eDOMTileNode: "eDOMTileNode",
        eDOMNodeTypeCnt: "eDOMNodeTypeCnt"
    }

    print(type_map.get(node_type, f"Unknown type ({node_type})"))


def main():
    try:
        test_filepath = "TestFiles/"

        # Instantiate Mako
        jaws_mako = IJawsMako.create("")
        IJawsMako.enableAllFeatures(jaws_mako)

        # Open PDF and get first page
        pdf_input = IPDFInput.create(jaws_mako)
        assembly = pdf_input.open(test_filepath + "Cheshire Cat.pdf")
        document = assembly.getDocument()
        fixed_page = document.getPage(0).getContent()

        # Walk the tree
        callback = NodeTreeCallback()
        fixed_page.walkTree(callback.getCallbackFunc(), callback.getPriv(), False, True)

    except MakoException as e:
        print(f"Exception thrown: {e.m_errorCode}: {e.m_msg}")
    except Exception as e:
        print(f"Exception thrown: {e}")


if __name__ == "__main__":
    main()
