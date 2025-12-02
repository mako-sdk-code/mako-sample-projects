/* -----------------------------------------------------------------------
 * <copyright file="WalkTree.cpp" company="Hybrid Software Helix Ltd">
 *  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Hybrid Software Helix Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 * </summary>
 * -----------------------------------------------------------------------
 */

#include <iostream>
#include <jawsmako/jawsmako.h>
#include <jawsmako/pdfinput.h>

using namespace JawsMako;
using namespace EDL;

// Forward declaration
bool VisitNode(void* priv, const IDOMNodePtr& node);

void DumpNodeInfo(IDOMNodePtr node)
{
    // Indent according to depth in DOM tree
    IDOMNodePtr p = node;
    while (p->getParentNode() != nullptr)
    {
        std::cout << "  ";
        p = p->getParentNode();
    }

    switch (node->getNodeType())
    {
    case eDOMNodeType::eDOMFixedPageNode:              std::cout << "eDOMFixedPageNode"; break;
    case eDOMNodeType::eDOMGroupNode:                  std::cout << "eDOMGroupNode"; break;
    case eDOMNodeType::eDOMCharPathGroupNode:          std::cout << "eDOMCharPathGroupNode"; break;
    case eDOMNodeType::eDOMTransparencyGroupNode:      std::cout << "eDOMTransparencyGroupNode"; break;
    case eDOMNodeType::eDOMGlyphsNode:                 std::cout << "eDOMGlyphsNode"; break;
    case eDOMNodeType::eDOMPathNode:                   std::cout << "eDOMPathNode"; break;
    case eDOMNodeType::eDOMFormNode:                   std::cout << "eDOMFormNode"; break;
    case eDOMNodeType::eDOMFormInstanceNode:           std::cout << "eDOMFormInstanceNode"; break;
    case eDOMNodeType::eDOMContentRootNode:            std::cout << "eDOMContentRootNode"; break;
    case eDOMNodeType::eDOMDocumentSequenceNode:       std::cout << "eDOMDocumentSequenceNode"; break;
    case eDOMNodeType::eDOMDocumentNode:               std::cout << "eDOMDocumentNode"; break;
    case eDOMNodeType::eDOMFixedDocumentNode:          std::cout << "eDOMFixedDocumentNode"; break;
    case eDOMNodeType::eDOMPageNode:                   std::cout << "eDOMPageNode"; break;
    case eDOMNodeType::eDOMCanvasNode:                 std::cout << "eDOMCanvasNode"; break;
    case eDOMNodeType::eDOMGlyphNode:                  std::cout << "eDOMGlyphNode"; break;
    case eDOMNodeType::eDOMRefNode:                    std::cout << "eDOMRefNode"; break;
    case eDOMNodeType::eDOMVisualRootNode:             std::cout << "eDOMVisualRootNode"; break;
    default:                                           std::cout << "Unknown node type"; break;
    }

    std::cout << std::endl;
}

// The callback that will be passed to walkTree()
bool VisitNode(void* priv, const IDOMNodePtr& node)
{
    DumpNodeInfo(node);
    return true; // Continue traversal
}

int main()
{
    try
    {
        U8String testFilepath = "../../TestFiles/";

        // Instantiate Mako
        IJawsMakoPtr jawsMako = IJawsMako::create("", "");
        IJawsMako::enableAllFeatures(jawsMako);

        // Open a PDF
        IPDFInputPtr pdfInput = IPDFInput::create(jawsMako);
        IDocumentAssemblyPtr assembly = pdfInput->open(testFilepath + "Cheshire Cat.pdf");

        // Get the first page’s fixed page content
        IDocumentPtr document = assembly->getDocument();
        IDOMFixedPagePtr fixedPage = document->getPage(0)->getContent();

        // Walk the DOM tree
        fixedPage->walkTree(&VisitNode, nullptr, false, true);
    }
    catch (IError& e)
    {
        const String errorFormatString = getEDLErrorString(e.getErrorCode());
        std::wcerr << L"Exception thrown: " << e.getErrorDescription(errorFormatString) << std::endl;
        return static_cast<int>(e.getErrorCode());
    }
    catch (std::exception& e)
    {
        std::cout << "std::exception: " << e.what() << std::endl;
    }

    return 0;
}
