/* -----------------------------------------------------------------------
 * <copyright file="RenderSeparationsToFrameBuffers.java" company="Hybrid Software Helix Ltd">
 *  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Hybrid Software Helix Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 * </summary>
 * -----------------------------------------------------------------------
 */

import java.io.*;
import java.math.BigInteger;
import java.nio.*;
import java.util.*;
import com.globalgraphics.JawsMako.jawsmakoIF.*;
import com.globalgraphics.JawsMako.jawsmakoIF.jawsmakoIF.*;

public class RenderSeparationsToFrameBuffers {
    public static void main(String[] args) {
        try {
            String testFilePath = "TestFiles/";

            if (args.length < 4) {
                System.out.println("Usage: java RenderSeparationsToFrameBuffers <source file> <spots to retain> <spots to ignore> <framebuffers=true/false>");
                return;
            }

            String inputFile = args[0];
            String spotsToRetain = args[1];
            String spotsToIgnore = args[2];
            String renderToFrameBuffers = args[3];

            if (!renderToFrameBuffers.equals("true") && !renderToFrameBuffers.equals("false")) {
                System.out.println("Parameter framebuffers must be 'true' or 'false' not " + renderToFrameBuffers);
                return;
            }

            IJawsMako mako = IJawsMako.create("", "");
            IJawsMako.enableAllFeatures(mako);
            var factory = mako.getFactory();

            // Input
            IPDFInput pdfInput = IPDFInput.create(mako);
            IDocumentAssembly assembly = pdfInput.open(testFilePath + inputFile);
            IPage page = assembly.getDocument().getPage();
            IDOMFixedPage fixedPage = page.getContent();

            // Set image dimensions + color space
            double resolution = 576.0;
            FRect bounds = new FRect(0, 0, page.getWidth(), page.getHeight());

            int pixelWidth = (int) Math.round(bounds.getDX() / 96.0 * resolution);
            int pixelHeight = (int) Math.round(bounds.getDY() / 96.0 * resolution);

            int depth = 8;
            IDOMColorSpaceDeviceCMYK testspace = IDOMColorSpaceDeviceCMYK.create(factory);

            // Find inks
            CEDLVectCInkInfo inks = IRendererTransform.findInks(mako, fixedPage);

            List<String> componentNames = new ArrayList<>();
            CEDLVectString ignoreSpotColorNames = new CEDLVectString();
            CEDLVectString retainSpotColorNames = new CEDLVectString();

            int numComponents = testspace.getNumComponents();
            for (int i = 0; i < numComponents; i++) {
                componentNames.add(testspace.getColorantName((byte) i));
            }

            for (int i = 0; i < inks.size(); i++) {
                var ink = inks.getitem(i);
                String inkName = ink.getInkName();
                if (spotsToIgnore.contains(inkName)) {
                    ignoreSpotColorNames.append(inkName);
                } else if (spotsToRetain.contains(inkName)) {
                    retainSpotColorNames.append(inkName);
                    componentNames.add(inkName);
                }
            }

            IJawsRenderer renderer = IJawsRenderer.create(mako);

            if (renderToFrameBuffers.equals("false")) {
                // Render using renderSeparations
                var images = renderer.renderSeparations(
                        fixedPage,
                        (short) depth,
                        testspace,
                        (short) 0,
                        bounds,
                        pixelWidth,
                        pixelHeight,
                        retainSpotColorNames,
                        IOptionalContent.Null(),
                        eOptionalContentEvent.eOCEView,
                        new CEDLVectString(),
                        false,
                        BigInteger.valueOf(0),
                        ignoreSpotColorNames
                );

                // Write outputs to TIFF
                String stem = new File(inputFile).getName().replaceFirst("[.][^.]+$", "");

                for (int j = 0; j < componentNames.size(); j++) {
                    String tiffFileName = stem + "_regular_" + componentNames.get(j) + ".tif";
                    IDOMTIFFImage.encode(mako, images.getitem(j), IOutputStream.createToFile(factory, tiffFileName));
                }
            }
            else {
                // Prepare frame buffers
                int numChannels = componentNames.size();
                ByteBuffer[] buffers = new ByteBuffer[numChannels];
                CEDLVectCFrameBufferInfo frameBuffers = new CEDLVectCFrameBufferInfo();

                for (byte i = 0; i < numChannels; i++) {
                    ByteBuffer buf = ByteBuffer.allocateDirect(pixelWidth * pixelHeight);
                    buf.order(ByteOrder.nativeOrder());
                    buffers[i] = buf;

                    IJawsRenderer.CFrameBufferInfo fbInfo = new IJawsRenderer.CFrameBufferInfo();
                    fbInfo.setBufferOfs(0);
                    fbInfo.setRowStride(pixelWidth);
                    fbInfo.setPixelStride(0);
                    frameBuffers.append(fbInfo);
                }

                // Render using renderSeparationsToFrameBuffers
                renderer.renderSeparationsToFrameBuffers(
                        fixedPage,
                        (short) depth,
                        true,
                        pixelWidth,
                        pixelHeight,
                        testspace,
                        buffers,
                        frameBuffers,
                        (short) 0,
                        bounds,
                        new CEDLVectWString(retainSpotColorNames.toArray()),
                        IOptionalContent.Null(),
                        eOptionalContentEvent.eOCEView,
                        new CEDLVectWString(),
                        false,
                        BigInteger.valueOf(0),
                        new CEDLVectWString(ignoreSpotColorNames.toArray())
                );

                // Write outputs to TIFFs
                String stem = new File(inputFile).getName().replaceFirst("[.][^.]+$", "");

                for (int j = 0; j < numChannels; j++) {
                    String tiffFileName = stem + "_frameBuffer_" + componentNames.get(j) + ".tif";

                    var pair = IDOMTIFFImage.createWriterAndImage(
                            mako,
                            IDOMColorSpaceDeviceGray.create(factory),
                            pixelWidth,
                            pixelHeight,
                            (short) depth,
                            96.0, 96.0,
                            IDOMTIFFImage.eTIFFCompression.eTCAuto,
                            IDOMTIFFImage.eTIFFPrediction.eTPNone,
                            eImageExtraChannelType.eIECNone,
                            false,
                            IInputStream.createFromFile(factory, testFilePath + inputFile),
                            IOutputStream.createToFile(factory, tiffFileName)
                    );

                    IImageFrameWriter frameWriter = pair.getFrameWriter();

                    ByteBuffer buffer = buffers[j];
                    byte[] scanline = new byte[pixelWidth];
                    for (int y = 0; y < pixelHeight; y++) {
                        buffer.position(y * pixelWidth);
                        buffer.get(scanline, 0, pixelWidth);
                        frameWriter.writeScanLine(scanline);
                    }
                    frameWriter.flushData();
                }
            }
            System.out.println("Rendering complete.");

        } catch (Exception e) {
            System.err.println("Exception: " + e.getMessage());
        }
    }
}
