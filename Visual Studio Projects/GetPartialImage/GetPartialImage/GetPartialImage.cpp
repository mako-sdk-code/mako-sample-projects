
/* -----------------------------------------------------------------------
 * <copyright file="GetPartialImage.cpp" company="Hybrid Software Helix Ltd">
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

using namespace JawsMako;
using namespace EDL;

IDOMImagePtr getPartialImage(const IJawsMakoPtr& mako, IDOMImagePtr image, const FRect& subImageRect);

int main()
{
    try
    {
        const auto mako = IJawsMako::create();
        mako->enableAllFeatures(mako);
        const U8String testFilePath = R"(..\..\TestFiles\)";
        const IDOMImagePtr image =
            IDOMJPEGImage::create(mako, IInputStream::createFromFile(mako, testFilePath + "WEV_086.JPG"));

        const auto partialImage = getPartialImage(mako, image, FRect(230, 230, 400, 250));
        IDOMPNGImage::encode(mako, partialImage, IOutputStream::createToFile(mako, "JustTheKayak.png"));
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

/// <summary>
 /// Extract part of an image, the area of which is described by an FRect where:
 ///   x,y are the top left corner, in pixels
 ///   dX, dY are the width and height
 /// </summary>
 /// <param name="mako"></param>
 /// <param name="image"></param>
 /// <param name="subImageRect"></param>
 /// <returns>The resulting IDOMImage</returns>
IDOMImagePtr getPartialImage(const IJawsMakoPtr& mako, IDOMImagePtr image, const FRect& subImageRect)
{
    // Get some details of the original image
    auto imageFrame = image->getImageFrame(mako);
    auto bps = imageFrame->getBPS();

    if (bps < 8)
    {
        image = IDOMFilteredImage::create(mako, image, IDOMImageBitScalerFilter::create(mako, 8));
        imageFrame = image->getImageFrame(mako);
        bps = 8;
    }
    else if (bps != 8 && bps != 16)
    {
        image = IDOMFilteredImage::create(mako, image, IDOMImageBitScalerFilter::create(mako, 16));
        imageFrame = image->getImageFrame(mako);
        bps = 16;
    }

    const auto colorSpace = imageFrame->getColorSpace();
    const auto stride = imageFrame->getRawBytesPerRow();
    const auto bpp = imageFrame->getNumChannels() * bps / 8;

    // Check the requested area is within the bounds of the original
    const auto originalImageRect = new FRect(0.0, 0.0, imageFrame->getWidth(), imageFrame->getHeight());
    if (!originalImageRect->containsRect(subImageRect))
        return {};

    IRAInputStreamPtr reader;
    IRAOutputStreamPtr writer;
    mako->getTempStore()->createTemporaryReaderWriterPair(reader, writer);
    const IInputStreamPtr inStream = IInputStream::createFromLz4Compressed(mako, reader);
    const IOutputStreamPtr outStream = IOutputStream::createToLz4Compressed(mako, writer);

    IImageFrameWriterPtr frameWriter;
    auto subImage = IDOMRawImage::createWriterAndImage(
        mako,
        frameWriter,
        colorSpace,
        static_cast<uint32>(subImageRect.dX),
        static_cast<uint32>(subImageRect.dY),
        bps,
        imageFrame->getXResolution(),
        imageFrame->getYResolution(),
        eImageExtraChannelType::eIECNone,
        inStream, outStream);

    CEDLSimpleBuffer rowBuffer(stride);

    // Move row pointer down to the first row of the partial image
    imageFrame->skipScanLines(static_cast<uint32>(subImageRect.y));

    // Copy the portion of the scanlines that we need
    for (uint32 i = 0; i < static_cast<uint32>(subImageRect.dY); i++)
    {
        imageFrame->readScanLine(&rowBuffer[0], rowBuffer.size());
        frameWriter->writeScanLine(&rowBuffer[static_cast<uint32>(subImageRect.x) * bpp]);
    }
    frameWriter->flushData();

    return subImage;
}
