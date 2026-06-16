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

#include <filesystem>
#include <iostream>
#include <jawsmako/jawsmako.h>
#include <jawsmako/pdfinput.h>

using namespace JawsMako;
using namespace EDL;

const U8String TEST_FILES_PATH = R"(..\..\..\..\TestFiles\)";

// Forward declaration
static bool VisitNode(void* priv, const IDOMNodePtr& node);

static void DumpNodeInfo(IDOMNodePtr node)
{
    // Indent according to depth in DOM tree
    auto p = node;
    while (p->getParentNode() != nullptr)
    {
        std::cout << "  ";
        p = p->getParentNode();
    }

    switch (node->getNodeType())
    {
    case eDOMFixedPageNode:              std::cout << "eDOMFixedPageNode"; break;
    case eDOMGroupNode:                  std::cout << "eDOMGroupNode"; break;
    case eDOMCharPathGroupNode:          std::cout << "eDOMCharPathGroupNode"; break;
    case eDOMTransparencyGroupNode:      std::cout << "eDOMTransparencyGroupNode"; break;
    case eDOMGlyphsNode:                 std::cout << "eDOMGlyphsNode"; break;
    case eDOMPathNode:                   std::cout << "eDOMPathNode"; break;
    case eDOMFormNode:                   std::cout << "eDOMFormNode"; break;
    case eDOMFormInstanceNode:           std::cout << "eDOMFormInstanceNode"; break;
    case eDOMContentRootNode:            std::cout << "eDOMContentRootNode"; break;
    case eDOMDocumentSequenceNode:       std::cout << "eDOMDocumentSequenceNode"; break;
    case eDOMDocumentNode:               std::cout << "eDOMDocumentNode"; break;
    case eDOMFixedDocumentNode:          std::cout << "eDOMFixedDocumentNode"; break;
    case eDOMPageNode:                   std::cout << "eDOMPageNode"; break;
    case eDOMCanvasNode:                 std::cout << "eDOMCanvasNode"; break;
    case eDOMGlyphNode:                  std::cout << "eDOMGlyphNode"; break;
    case eDOMRefNode:                    std::cout << "eDOMRefNode"; break;
    case eDOMVisualRootNode:             std::cout << "eDOMVisualRootNode"; break;
    default:                                           std::cout << "Unknown node type"; break;
    }

    std::cout << '\n';
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
        // Instantiate Mako
        auto jawsMako = IJawsMako::create("", "");
        IJawsMako::enableAllFeatures(jawsMako);

        // Open a PDF
        auto pdfInput = IPDFInput::create(jawsMako);
        auto assembly = pdfInput->open(TEST_FILES_PATH + "Cheshire Cat.pdf");

        // Get the first pages fixed page content
        auto document = assembly->getDocument();
        auto fixedPage = document->getPage()->getContent();

        // Walk the DOM tree
        fixedPage->walkTree(&VisitNode, nullptr, false, true);
    }
    catch (IError& e)
    {
        const String errorFormatString = getEDLErrorString(e.getErrorCode());
        std::wcerr << L"Exception thrown: " << e.getErrorDescription(errorFormatString) << '\n';
        return static_cast<int>(e.getErrorCode());
    }
    catch (std::exception& e)
    {
        std::cout << "std::exception: " << e.what() << '\n';
    }

    return 0;
}
