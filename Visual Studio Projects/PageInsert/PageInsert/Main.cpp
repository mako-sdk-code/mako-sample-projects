
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
#include <jawsmako/pdfpage.h>
#include <chrono>

using namespace JawsMako;
using namespace EDL;

int main(int argc, char* argv[])
{

    U8String testFilePath = R"(..\..\TestFiles\)";
    if (argc < 3)
    {
        std::wcerr << "Usage: " << argv[0] << " <input> <output> <cover>" << std::endl;
        return -1;
    }

    try
    {
        const auto mako = IJawsMako::create();
        mako->enableAllFeatures(mako);

        // Define input/output paths
        const U8String fileThatNeedsACover = testFilePath + argv[1];
        const U8String fileThatNowHasACover = argv[2];
        const U8String coverPageFile = testFilePath + argv[3];
        
        // Start timer
        auto start = std::chrono::high_resolution_clock::now();

        // File we want to insert into
        auto inputStream = IInputStream::createFromFile(mako, fileThatNeedsACover);

        // File we want to get our cover page from
        auto insertStream = IInputStream::createFromFile(mako, coverPageFile);

        // Create inserter
        auto pageInserter = IPDFPageInserter::create(mako, inputStream);

        // Do the insertion – page numbers are zero-based
        pageInserter->insert(insertStream, 0, 0, 1);

        // And save
        pageInserter->save(IOutputStream::createToFile(mako, fileThatNowHasACover));

        // Stop timer and report
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
        std::wcout << L"Processing time: " << (duration.count() / 1000.0) << L" seconds." << std::endl;

        
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