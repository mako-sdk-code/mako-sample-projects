/*
 * -----------------------------------------------------------------------
 * <copyright file="MakoPartialImage.java" company="Hybrid Software Helix Ltd">
 *  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Hybrid Software Helix Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 * </summary>
 * -----------------------------------------------------------------------
 */

import com.globalgraphics.JawsMako.jawsmakoIF.*;

public class MakoPartialImage {

    public static void main(String[] args) {
        try {
            // Instantiate Mako
            IJawsMako mako = IJawsMako.create();
            IJawsMako.enableAllFeatures(mako);
            var factory = mako.getFactory();

            String testFilePath = "TestFiles/";
            IDOMImage image = IDOMJPEGImage.create(
                    factory,
                    IInputStream.createFromFile(factory, testFilePath + "WEV_086.JPG")
            );

            // Extract a portion of the image (the kayak area)
            FRect subImageRect = new FRect(230, 230, 400, 250);
            IDOMImage partialImage = getPartialImage(mako, image, subImageRect);

            IDOMPNGImage.encode(
                    mako,
                    partialImage,
                    IOutputStream.createToFile(factory, "JustTheKayak.png")
            );
        }
        catch (Exception e) {
            System.out.println("Exception thrown: " + e);
        }
    }

    /**
     * Extract part of an image, the area of which is described by an FRect where:
     * x,y are the top left corner, in pixels
     * dX, dY are the width and height
     */
    static IDOMImage getPartialImage(IJawsMako mako, IDOMImage image, FRect subImageRect) {
        var factory = mako.getFactory();
        IImageFrame imageFrame = image.getImageFrame(factory);
        int bps = imageFrame.getBPS();

        // Scale bits per sample to 8 or 16 if needed
        if (bps < 8) {
            image = IDOMFilteredImage.create(factory, image, IDOMImageBitScalerFilter.create(factory, (short)8));
            imageFrame = image.getImageFrame(factory);
            bps = 8;
        } else if (bps != 8 && bps != 16) {
            image = IDOMFilteredImage.create(factory, image, IDOMImageBitScalerFilter.create(factory, (short)16));
            imageFrame = image.getImageFrame(factory);
            bps = 16;
        }

        IDOMColorSpace colorSpace = imageFrame.getColorSpace();
        int stride = Math.toIntExact(imageFrame.getRawBytesPerRow());
        int bpp = imageFrame.getNumChannels() * bps / 8;

        // Check requested region is inside bounds
        FRect originalRect = new FRect(0.0, 0.0, imageFrame.getWidth(), imageFrame.getHeight());
        if (!originalRect.containsRect(subImageRect)) {
            return IDOMImage.Null();
        }

        // Create temp reader/writer
        var temp = mako.getTempStore().createTemporaryReaderWriter();
        IInputStream inStream = IInputStream.createFromLz4Compressed(factory, temp.toIInputStream());
        IOutputStream outStream = IOutputStream.createToLz4Compressed(factory, temp.toIOutputStream());

        // Create the new raw image and writer
        var imageAndWriter = IDOMRawImage.createWriterAndImage(
                mako,
                colorSpace,
                (long) subImageRect.getDX(),
                (long) subImageRect.getDY(),
                (short) bps,
                imageFrame.getXResolution(),
                imageFrame.getYResolution(),
                eImageExtraChannelType.eIECNone,
                inStream,
                outStream
        );

        IDOMImage subImage = imageAndWriter.getDomImage();
        IImageFrameWriter frameWriter = imageAndWriter.getFrameWriter();

        // Prepare buffers
        byte[] rowBuffer = new byte[stride];
        byte[] targetRowBuffer = new byte[(int) subImageRect.getDX() * bpp];

        // Move to the starting scanline
        imageFrame.skipScanLines((long) subImageRect.getY());

        // Copy only the desired region
        for (int i = 0; i < (int) subImageRect.getDY(); i++) {
            imageFrame.readScanLine(rowBuffer);
            System.arraycopy(rowBuffer, (int) subImageRect.getX() * bpp, targetRowBuffer, 0, targetRowBuffer.length);
            frameWriter.writeScanLine(targetRowBuffer);
        }

        frameWriter.flushData();
        return subImage;
    }
}
