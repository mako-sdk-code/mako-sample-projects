/* --------------------------------------------------------------------------------
 *  <copyright file="GetPartialImage.cs" company="Hybrid Software Helix Ltd">
 *    Copyright (c) 2025 Hybrid Software Helix Ltd. All rights reserved.
 *  </copyright>
 *  <summary>
 *    This example is provided on an "as is" basis and without warranty of any kind.
 *    Hybrid Software Helix Ltd. does not warrant or make any representations
 *    regarding the use or results of use of this example.
 *  </summary>
 * ---------------------------------------------------------------------------------
 */

using JawsMako;

namespace GetPartialImageCS;

internal class GetPartialImageCS
{
    private const string TestFilesPath = @"..\..\..\..\..\..\TestFiles\";

    static int Main()
    {
        try
        {
            using var mako = IJawsMako.create();
            IJawsMako.enableAllFeatures(mako);
            using var image = IDOMJPEGImage.create(mako,
                IInputStream.createFromFile(mako, TestFilesPath + "WEV_086.JPG"));

            using var partialImage = GetPartialImage(mako, image, new FRect(230, 230, 400, 250));
            IDOMPNGImage.encode(mako, partialImage, IOutputStream.createToFile(mako, "JustTheKayak.png"));
        }
        catch (MakoException e)
        {
            Console.Error.WriteLine($"Mako exception thrown: {e.m_msg}");
            return 1;
        }
        catch (Exception e)
        {
            Console.Error.WriteLine($"Exception thrown: {e}");
            return 1;
        }

        return 0;
    }

    /// <summary>
    /// Extract part of an image, the area of which is described by an FRect where:
    ///   x,y are the top left corner, in pixels
    ///   dX, dY are the width and height
    /// </summary>
    /// <param name="mako"></param>
    /// <param name="image"></param>
    /// <param name="subImageRect"></param>
    /// <returns>The resulting IDOMImage</returns>
    private static IDOMImage GetPartialImage(IJawsMako mako, IDOMImage image, FRect subImageRect)
    {
        // Get some details of the original image
        var imageFrame = image.getImageFrame(mako);
        var bps = imageFrame.getBPS();

        if (bps < 8)
        {
            image = IDOMFilteredImage.create(mako, image, IDOMImageBitScalerFilter.create(mako, 8));
            imageFrame = image.getImageFrame(mako);
            bps = 8;
        }
        else if (bps != 8 && bps != 16)
        {
            image = IDOMFilteredImage.create(mako, image, IDOMImageBitScalerFilter.create(mako, 16));
            imageFrame = image.getImageFrame(mako);
            bps = 16;
        }

        using var colorSpace = imageFrame.getColorSpace();
        var stride = imageFrame.getRawBytesPerRow();
        var bpp = imageFrame.getNumChannels() * bps / 8;

        // Check the requested area is within the bounds of the original
        using var originalImageRect = new FRect(0.0, 0.0, imageFrame.getWidth(), imageFrame.getHeight());
        if (!originalImageRect.containsRect(subImageRect))
            return IDOMImage.Null();

        var (reader, writer) = mako.getTempStore().createTemporaryReaderWriterTuple();
        using var inStream = IInputStream.createFromLz4Compressed(mako, reader);
        using var outStream = IOutputStream.createToLz4Compressed(mako, writer);

        using var imageAndWriter = IDOMRawImage.createWriterAndImage(
            mako,
            colorSpace,
            (uint)subImageRect.dX,
            (uint)subImageRect.dY,
            bps,
            imageFrame.getXResolution(),
            imageFrame.getYResolution(),
            eImageExtraChannelType.eIECNone,
            inStream, outStream);

        var subImage = imageAndWriter.domImage;
        using var frameWriter = imageAndWriter.frameWriter;

        imageAndWriter.domImage = null;
        imageAndWriter.frameWriter = null;

        var rowBuffer = new byte[stride];
        var targetRowBuffer = new byte[(uint)subImageRect.dX * bpp];

        // Move row pointer down to the first row of the partial image
        imageFrame.skipScanLines((uint)subImageRect.y);

        // Copy the portion of the scanlines that we need
        for (uint i = 0; i < (uint)subImageRect.dY; i++)
        {
            imageFrame.readScanLine(rowBuffer);
            Array.Copy(rowBuffer, (uint)subImageRect.x * bpp, targetRowBuffer, 0, targetRowBuffer.Length);
            frameWriter.writeScanLine(targetRowBuffer);
        }
        frameWriter.flushData();

        return subImage;
    }
}