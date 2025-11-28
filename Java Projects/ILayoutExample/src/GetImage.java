/* -----------------------------------------------------------------------
 * <copyright file="GetImage.java" company="Hybrid Software Helix Ltd">
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

import java.io.File;

public class GetImage {
    public IDOMImage image;
    public double width;
    public double height;

    GetImage(IJawsMako mako, String imageFile, double requiredWidth, double requiredHeight) throws Exception {
        File fileInfo = new File(imageFile);
        if (!fileInfo.exists()) {
            throw new Exception("Image file {imageFile} not found.");
        }

        image = null;
        if (fileInfo.getName().toLowerCase().contains(".jpg")) {
            image = IDOMJPEGImage.create(mako.getFactory(), IInputStream.createFromFile(mako.getFactory(), imageFile));
        }

        if (fileInfo.getName().toLowerCase().contains(".png")) {
            image = IDOMPNGImage.create(mako.getFactory(), IInputStream.createFromFile(mako.getFactory(), imageFile));
        }

        if (fileInfo.getName().toLowerCase().contains(".tif")) {
            image = IDOMTIFFImage.create(mako.getFactory(), IInputStream.createFromFile(mako.getFactory(), imageFile));
        }

        if (image == null) {
            throw new Exception("Image file " + imageFile + " could not be loaded.");
        }

        var frame = image.getImageFrame(mako.getFactory());
        double imageWidth = frame.getWidth();
        double imageHeight = frame.getHeight();
        double aspectRatio = imageWidth / imageHeight;

        // If requested dimensions are both zero, return the actual size
        if (requiredWidth == 0.0 && requiredHeight == 0.0) {
            width = imageWidth;
            height = imageHeight;
        }

        // If requested dimensions are both non-zero, return those values (do nothing)
        if (requiredWidth > 0.0 && requiredHeight > 0.0) {
            width = requiredWidth;
            height = requiredHeight;
        }

        // Otherwise, calculate the missing dimension
        if (requiredHeight == 0.0) {
            width = requiredWidth;
            height = requiredWidth / aspectRatio;
        } else {
            width = requiredHeight * aspectRatio;
            height = requiredHeight;
        }
    }
}