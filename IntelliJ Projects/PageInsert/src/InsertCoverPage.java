/* -----------------------------------------------------------------------
 * <copyright file="InsertCoverPage.java" company="Hybrid Software Helix Ltd">
 *  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  Inserts a cover page into an existing PDF and measures processing time.
 * </summary>
 * -----------------------------------------------------------------------
 */

import com.globalgraphics.JawsMako.jawsmakoIF.*;

public class InsertCoverPage {
    public static void main(String[] args) {
        try {

            if (args.length < 3) {
                System.err.println("Usage: InsertCoverPage <input> <output> <cover>");
            }

            // Start stopwatch
            long startTime = System.currentTimeMillis();

            // Instantiate Mako
            IJawsMako jawsMako = IJawsMako.create();
            IJawsMako.enableAllFeatures(jawsMako);
            var factory = jawsMako.getFactory();

            // Define file paths
            String testFilePath = "TestFiles/";
            String fileThatNeedsACover = testFilePath + args[0];
            String fileThatNowHasACover = args[1];
            String coverPageFile = testFilePath + args[2];


            // File we want to insert into
            IInputStream inputStream = IInputStream.createFromFile(factory, fileThatNeedsACover);

            // File we want to get our cover page from
            IInputStream insertStream = IInputStream.createFromFile(factory, coverPageFile);

            // Create PDF Page Inserter
            IPDFPageInserter pageInserter = IPDFPageInserter.create(jawsMako, inputStream);

            // Perform insertion (page numbers are zero-based)
            pageInserter.insert(insertStream, 0, 0, 1);

            // Save the result
            IOutputStream outputStream = IOutputStream.createToFile(factory, fileThatNowHasACover);
            pageInserter.save(outputStream);

            // Stop stopwatch and report
            long endTime = System.currentTimeMillis();
            double elapsedSeconds = (endTime - startTime) / 1000.0;
            System.out.printf("Processing time: %.4f seconds.%n", elapsedSeconds);
        }
        catch (Exception e) {
            System.err.println("Unexpected exception: " + e);
        }
    }
}
