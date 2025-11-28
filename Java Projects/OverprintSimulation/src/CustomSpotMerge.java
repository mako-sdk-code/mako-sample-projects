/*
 * -----------------------------------------------------------------------
 * <copyright file="CustomSpotMerge.java" company="Hybrid Software Helix Ltd">
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
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.*;
import com.globalgraphics.JawsMako.jawsmakoIF.*;

public class CustomSpotMerge {

    public static void main(String[] args) {
        // Adjust to your test files folder
        String testFilePath = "TestFiles/";

        try {
            IJawsMako mako = IJawsMako.create();
            IJawsMako.enableAllFeatures(mako);
            IEDLClassFactory factory = mako.getFactory();

            // Load the document
            IPDFInput pdfInput = IPDFInput.create(mako);
            IDocumentAssembly docAsm = pdfInput.open(testFilePath + "Robots Plus Process Colors.pdf");
            IDocument doc = docAsm.getDocument();

            for (int pageIndex = 0; pageIndex < doc.getNumPages(); pageIndex++) {
                IDOMFixedPage fixedPage = doc.getPage(pageIndex).getContent();

                // Page bounds & raster geometry
                FRect bounds = new FRect(0, 0, fixedPage.getWidth(), fixedPage.getHeight());
                double resolution = 150.0;
                int pixelWidth = (int) Math.round(bounds.getDX() / 96.0 * resolution);
                int pixelHeight = (int) Math.round(bounds.getDY() / 96.0 * resolution);

                // Colorspace must be CMYK for spot merging
                IDOMColorSpaceDeviceCMYK cmyk = IDOMColorSpaceDeviceCMYK.create(factory);

                // Find inks and build spot list
                CEDLVectColorantInfo spots =
                        IRendererTransform.inkInfoToColorantInfo(mako,
                                IRendererTransform.findInks(mako, fixedPage), cmyk);

                CEDLVectWString spotNames = new CEDLVectWString();
                for (int i = 0; i < spots.size(); i++)
                    spotNames.append(spots.getitem(i).getName());

                int numProcess = cmyk.getNumComponents(); // 4
                int numSpots = (int) spots.size();
                int numBuffers = numProcess + numSpots;

                // Prepare frame buffers
                ByteBuffer[] buffers = new ByteBuffer[numBuffers];
                CEDLVectCFrameBufferInfo fb = new CEDLVectCFrameBufferInfo();

                for (int i = 0; i < numBuffers; i++) {
                    ByteBuffer buf = ByteBuffer.allocateDirect(pixelWidth * pixelHeight);
                    buf.order(ByteOrder.nativeOrder());
                    buffers[i] = buf;

                    IJawsRenderer.CFrameBufferInfo info = new IJawsRenderer.CFrameBufferInfo();
                    info.setBufferOfs(0);
                    info.setRowStride(pixelWidth);  // bytes per row
                    info.setPixelStride(0);         // tightly packed
                    fb.append(info);
                }

                // Render true separations
                IJawsRenderer renderer = IJawsRenderer.create(mako);
                renderer.renderSeparationsToFrameBuffers(
                        fixedPage,
                        (short) 8,
                        true,
                        pixelWidth,
                        pixelHeight,
                        cmyk,
                        buffers,
                        fb,
                        (short) 0,
                        bounds,
                        spotNames
                );

                // Create writer and image
                var pair = IDOMRawImage.createWriterAndImage(mako, cmyk, pixelWidth, pixelHeight,
                        (short) 8, resolution, resolution);

                IImageFrameWriter frameWriter = pair.getFrameWriter();

                // Spot components to merge
                CEDLVectVectFloat components = new CEDLVectVectFloat();
                for (int i = 0; i < numSpots; i++) {
                    CEDLVectFloat comps = new CEDLVectFloat();
                    CEDLVectFloat vals = spots.getitem(i).getComponents();
                    for (int c = 0; c < 4; c++)
                        comps.append(vals.getitem(c));
                    components.append(comps);
                }

                // Merge each spot buffer with process values
                final float inv255 = 1.0f / 255.0f;
                byte[] scanline = new byte[pixelWidth * numProcess];
                for (int y = 0; y < pixelHeight; y++) {
                    int rowStart = y * pixelWidth;
                    for (int x = 0; x < pixelWidth; x++) {
                        for (int c = 0; c < numProcess; c++) {
                            int idx = x * numProcess + c;
                            scanline[idx] = buffers[c].get(rowStart + x);
                            for (int i = 0; i < numSpots; i++) {
                                float spotVal = (buffers[numProcess + i].get(rowStart + x) & 0xFF) * inv255;
                                float currentVal = (scanline[idx] & 0xFF) * inv255;
                                float newVal = 1.0f - (1.0f - components.getitem(i).getitem(c) * spotVal) * (1.0f - currentVal);
                                scanline[idx] = (byte) (newVal * 255.0f + 0.5f);
                            }
                        }
                    }
                    frameWriter.writeScanLine(scanline);
                }

                frameWriter.flushData();

                // Convert to RGB and save as JPEG
                IDOMColorSpaceDeviceRGB rgb = IDOMColorSpaceDeviceRGB.create(factory);
                IDOMImageColorConverterFilter cc =
                        IDOMImageColorConverterFilter.create(factory, rgb,
                                eRenderingIntent.eRelativeColorimetric,
                                eBlackPointCompensation.eBPCDefault);
                IDOMFilteredImage filtered = IDOMFilteredImage.create(factory, pair.getDomImage(), cc);

                String outJpeg = String.format("output_%d.jpg", pageIndex);
                IDOMJPEGImage.encode(mako, filtered,
                        IOutputStream.createToFile(factory, outJpeg));

                System.out.println("Wrote: " + outJpeg);
            }
        }
        catch (Exception ex) {
            ex.printStackTrace();
            System.exit(1);
        }
    }
}
