
/* -----------------------------------------------------------------------
 * <copyright file="OverprintMethods.cpp" company="Global Graphics Software Ltd">
 *  Copyright (c) 2025 Global Graphics Software Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Global Graphics Software Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 * </summary>
 * -----------------------------------------------------------------------
 */

#include <filesystem>
#include <iostream>

#include <jawsmako/jawsmako.h>
#include <jawsmako/pdfoutput.h>

using namespace JawsMako;
using namespace EDL;

const U8String TEST_FILES_PATH = R"(..\..\..\..\TestFiles\)";

int main()
{
    try
    {
        const auto mako = IJawsMako::create();
        IJawsMako::enableAllFeatures(mako);
        const auto assembly = IInput::create(mako, eFFPDF)->open(TEST_FILES_PATH + "CMYK_Circles 1.pdf");
        const auto document = assembly->getDocument();

        // Apply overprint to all path nodes in all pages
        for (uint32 pageIndex = 0; pageIndex < document->getNumPages(); ++pageIndex) {
            auto page = document->getPage(pageIndex);
            auto fixedPage = page->edit();

            CEDLVector<IDOMNodePtr> pathNodes;
            fixedPage->findChildrenOfType(eDOMPathNode, pathNodes, true);

            for (auto& node : pathNodes) {
	            if (auto path = edlobj2IDOMPathNode(node))
                {
	                // Apply overprint fill using the new API
	                path->setFillOverprints(true);
	                // If needed, apply overprint stroke
	                //path->setStrokeOverprints(true);
	                path->setOverprintMode(true);
                }
            }
        }

        IPDFOutput::create(mako)->writeAssembly(assembly, "test.pdf");
    }
    catch (IError& e)
    {
        const String errorFormatString = getEDLErrorString(e.getErrorCode());
        std::wcerr << L"Exception thrown: " << e.getErrorDescription(errorFormatString) << '\n';
        return static_cast<int>(e.getErrorCode());
    }
    catch (std::exception& e)
    {
        std::wcerr << L"std::exception thrown: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
