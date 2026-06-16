/* --------------------------------------------------------------------------------
 *  <copyright file="PageInsert.cs" company="Hybrid Software Helix Ltd">
 *    Copyright (c) 2025 Hybrid Software Helix Ltd. All rights reserved.
 *  </copyright>
 *  <summary>
 *    This example is provided on an "as is" basis and without warranty of any kind.
 *    Hybrid Software Helix Ltd. does not warrant or make any representations
 *    regarding the use or results of use of this example.
 *  </summary>
 * ---------------------------------------------------------------------------------
 */

using System.Diagnostics;
using JawsMako;
using static JawsMako.jawsmakoIF_csharp;

namespace PageInsertCS;

internal class PageInsert
{
    private const string TestFilesPath = @"..\..\..\..\..\..\TestFiles\";

    static int Main(string[] args)
    {
        if (args.Length != 3)
        {
            Console.Error.WriteLine($"Usage: {AppDomain.CurrentDomain.FriendlyName} <input> <output> <cover>");
            return 1;
        }

        var fileThatNeedsACover = TestFilesPath + args[0];
        var fileThatNowHasACover = args[1];
        var coverPageFile = TestFilesPath + args[2];

        try
        {
            // Create Mako instance
            var mako = IJawsMako.create();
            IJawsMako.enableAllFeatures(mako);

            var stopWatch = new Stopwatch();
            stopWatch.Start();

            // File we want to insert into
            using var inputStream = IInputStream.createFromFile(mako, fileThatNeedsACover);

            // File we want to get our cover page from 
            using var insertStream = IInputStream.createFromFile(mako, coverPageFile);

            // Create inserter
            using var pageInserter = IPDFPageInserter.create(mako, inputStream);

            // Do the insertion - page numbers are zero-based
            pageInserter.insert(insertStream, 0, 0, 1);

            // And save
            pageInserter.save(IOutputStream.createToFile(mako, fileThatNowHasACover));

            Console.WriteLine($"Processing time: {stopWatch.ElapsedMilliseconds / 1000.0:F4} seconds.");
            stopWatch.Stop();
        }
        catch (MakoException e)
        {
            Console.WriteLine($"Mako exception thrown: {getEDLErrorString(e.m_errorCode)}");
            return 1;
        }

        catch (Exception e)
        {
            Console.WriteLine($"Exception thrown: {e}");
            return 1;
        }
        return 0;
    }
}