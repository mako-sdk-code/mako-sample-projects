
/* -----------------------------------------------------------------------
 * <copyright file="Main.cpp" company="Hybrid Software Helix Ltd">
 *  Copyright (c) 2025 Hybrid Software Helix Ltd. All rights reserved.
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
#include <jawsmako/pdfoutput.h>
#include "TextTransformImplementation.h"

using namespace JawsMako;
using namespace EDL;

int main(int argc, char* argv[])
{

    U8String testFilePath = R"(..\..\TestFiles\)";

    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <input> <output> <inkValue>" << std::endl;
        return 1;
    }

    U8String inputFile = argv[1];
    U8String outputFile = argv[2];
    float inkValue = atof(argv[3]);


    try
    {
        const auto mako = IJawsMako::create();
        mako->enableAllFeatures(mako);

        TextTransformImplementation implementation(mako, inkValue);
        const ITransformPtr textModifier = ICustomTransform::create(mako, &implementation);

        const auto assembly = IInput::create(mako, eFFPDF)->open(testFilePath + inputFile);
        const auto documentPtr = assembly->getDocument();

        const auto pageCount = documentPtr->getNumPages();

        std::wcout << L"Processing " << pageCount << " pages..." << std::endl;

        for (uint32 pageIndex = 0; pageIndex < pageCount; pageIndex++)
        {
            // Modify the page
            const auto page = documentPtr->getPage(pageIndex);
            textModifier->transformPage(page);
            page->release();
        }

        std::wcout << std::endl << L"Writing output file ..." << std::endl;

        const IPDFOutputPtr pdfOutput = IPDFOutput::create(mako);
        pdfOutput->writeAssembly(assembly, outputFile.c_str());

    }
    catch (IError& e)
    {
        const String errorFormatString = getEDLErrorString(e.getErrorCode());
        std::wcerr << L"Exception thrown: " << e.getErrorDescription(errorFormatString) << std::endl;
        return static_cast<int>(e.getErrorCode());
    }
    catch (std::exception& e)
    {
        std::wcerr << L"std::exception thrown: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}