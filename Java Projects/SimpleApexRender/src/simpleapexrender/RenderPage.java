/*
 * Copyright (C) 2025 Global Graphics Software Ltd. All rights reserved.
 */

package simpleapexrender;


import java.util.*;
import java.nio.*;
import com.globalgraphics.JawsMako.jawsmakoIF.*;
import com.globalgraphics.JawsMako.jawsmakoIF.jawsmakoIF.*;


public class RenderPage
{
    public RenderPage(IApexRenderer renderer,
                      ApexRenderParams renderParams,
                      IPage page,
                      IOptionalContent optionalContent,
                      int pageNumInFile
                      )
    {
        this.pageNumInFile      = pageNumInFile;
        this.errored            = false;
        this.errorString        = null;
        this.page               = page;
        this.optionalContent    = optionalContent;
        this.renderParams       = renderParams;
        this.renderer           = renderer;
        this.jawsMako           = renderParams.m_jawsMako;
        this.factory            = jawsMako.getFactory();
        this.fetchTimer         = new StopWatch("Fetch page ");
        this.renderPageTimer    = new StopWatch("Render page");
        
        this.pixelStride        = 0;
        this.rowStride          = 0;
        this.frameBufSize       = 0; 
    }

    // Set the common renderSpec paramters given the page, cropBox, and the parameters given
    // at the command line
    private void setCommonRenderParams (CRenderSpec renderSpec, 
                                        IDOMFixedPage content, 
                                        FRect cropBox)
    {
        // If we are being asked to perform overprint preview, find all the inks present on the page
        CEDLVectColorantInfo mergeSpots = new CEDLVectColorantInfo();
        if (renderParams.m_overprintPreview)
        {
            FindSpots.findAllSpots(jawsMako, content, renderParams.m_finalSpace, mergeSpots);
        }

        // Decide the size of the resulting rendered image. Here we'll just
        // render the crop box area.
        int   pixelWidth    = (int) (cropBox.getDX() * renderParams.m_xResolution / 96.0 + 0.5);
        int   pixelHeight   = (int) (cropBox.getDY() * renderParams.m_yResolution / 96.0 + 0.5);

        renderSpec.setProcessSpace(renderParams.m_processSpace);
        renderSpec.setWidth(pixelWidth);
        renderSpec.setHeight(pixelHeight);
        renderSpec.setDepth(renderParams.m_depth);
        renderSpec.setAaFactor(renderParams.m_aaFactor);
        renderSpec.setAlpha(renderParams.m_alpha);
        renderSpec.setSourceRect(cropBox);
        if (renderParams.m_ignoreIntercepts)
        {
            CColorManagerConfig overrideCmmConfig = CColorManagerConfig.create ();
            switch (renderParams.m_processSpace.getNumComponents())
            {
                case 1:
                    if (renderParams.m_processSpace.getColorSpaceType() != IDOMColorSpace.eColorSpaceType.eDeviceGray)
                    {
                        overrideCmmConfig.setDeviceGrayIntercept(renderParams.m_processSpace);
                    }
                    break;
                case 3:
                    if (renderParams.m_processSpace.getColorSpaceType() != IDOMColorSpace.eColorSpaceType.eDeviceRGB)
                    {
                        overrideCmmConfig.setDeviceRGBIntercept(renderParams.m_processSpace);
                    }
                    break;
                case 4:
                    if (renderParams.m_processSpace.getColorSpaceType() != IDOMColorSpace.eColorSpaceType.eDeviceCMYK)
                    {
                        overrideCmmConfig.setDeviceCMYKIntercept(renderParams.m_processSpace);
                    }
                    break;
                default:
                    break;
            }
            renderSpec.setOverrideCmmConfig(overrideCmmConfig);
        }
        renderSpec.setUse16BitInternalRendering(renderParams.m_use16BitInternalRendering);

        if (renderParams.m_optionalContentEvent != eOptionalContentEvent.eOCEUnknown)
        {
            renderSpec.setOptionalContent(optionalContent);
            renderSpec.setOptionalContentEvent(renderParams.m_optionalContentEvent);
        }

        CEDLVectIPostProcessSpec postProcesses = new CEDLVectIPostProcessSpec();

        // If final space differs from the process space, add a post process
        // to perform that conversion.
        if (renderParams.m_finalSpace != renderParams.m_processSpace)
        {
            postProcesses.append (CColorConversionPostProcessSpec.create (renderParams.m_finalSpace));
        }

        // And if we're merging spots, add the final post process for that
        if (! mergeSpots.empty ())
        {
            postProcesses.append (CSpotMergePostProcessSpec.create (mergeSpots));
        }

        renderSpec.setPostProcesses (postProcesses);

        // Convenience calculations
        
        // Allocate an 8bpc frame buffer
        short numChannels = renderParams.m_finalSpace.getNumComponents();
        if (renderParams.m_alpha)
        {
            numChannels++;
        }

        pixelStride     = numChannels * renderParams.m_depth / 8;
        rowStride       = pixelWidth * pixelStride;
        frameBufSize    = (int) (rowStride * pixelHeight);
    }
    
    public void RenderToFrameBuffer()
    {
        try
        {
            fetchTimer.start();
            IDOMFixedPage content = page.getContent();
            fetchTimer.end(1);

            // Decide the size of the resulting rendered image. Here we'll just
            // render the crop box area.
            FRect cropBox       = page.getCropBox();

            // Populate the render spec
            CFrameBufferRenderSpec renderSpec = new CFrameBufferRenderSpec();

            setCommonRenderParams (renderSpec, content, cropBox);

            renderSpec.setHostEndian(false);
            renderSpec.setRowStride(rowStride);

            if (frameBufSize < 0) 
                throw new Exception ("Integer overflow getting framebuffer size");
            
            byte[]      frameBuffer = new byte [frameBufSize];
            ByteBuffer  byteBuffer = ByteBuffer.allocateDirect (frameBufSize);

            renderPageTimer.start();
            renderer.render(content, renderSpec, byteBuffer);

            // Copy the result
            byteBuffer.get (frameBuffer);
            
            // We include the above call to retrieve the data from the direct
            // buffer because we want to measure this extra step as well.
            renderPageTimer.end(1);

            // Done with the page
            page.revert();
            page.release();
            

            // Create the path to the output
            String outPath = String.format(renderParams.m_outputFilePath, pageNumInFile);

            // Create the output image frame
            IRAInputStream readStream   = IInputStream.createFromFile(factory, outPath);
            IRAOutputStream writeStream = IOutputStream.createToFile(factory, outPath);

            eImageExtraChannelType extraChannel = renderParams.m_alpha ? eImageExtraChannelType.eIECAlpha
                                                                       : eImageExtraChannelType.eIECNone;

            Pair_IDOMIMage_IImageFrameWriter pairImageFrame = null;

            if (renderParams.m_renderedFormat == eRenderedFormat.eRFNone)
            {
            }
            else if (renderParams.m_renderedFormat == eRenderedFormat.eRFPNG)
            {
                // PNG
                pairImageFrame = IDOMPNGImage.createWriterAndImage(jawsMako,
                                                                    renderParams.m_finalSpace,
                                                                    renderSpec.getWidth(), renderSpec.getHeight(),
                                                                    renderParams.m_depth,
                                                                    renderParams.m_xResolution,
                                                                    renderParams.m_yResolution,
                                                                    extraChannel,
                                                                    readStream, writeStream);
            }
            else if (renderParams.m_renderedFormat == eRenderedFormat.eRFTIFF)
            {
                // TIFF
                pairImageFrame = IDOMTIFFImage.createWriterAndImage(jawsMako,
                                                                    renderParams.m_finalSpace,
                                                                    renderSpec.getWidth(), renderSpec.getHeight(),
                                                                    renderParams.m_depth,
                                                                    renderParams.m_xResolution,
                                                                    renderParams.m_yResolution,
                                                                    IDOMTIFFImage.eTIFFCompression.eTCAuto,
                                                                    IDOMTIFFImage.eTIFFPrediction.eTPNone,
                                                                    extraChannel,
                                                                    renderParams.m_bigTIFF,
                                                                    readStream, writeStream);
            }
            else
            {
                // Can't write with an extra channel
                if (extraChannel != eImageExtraChannelType.eIECNone)
                {
                    throw new MakoException (EDLErrorCode.JM_ERR_GENERAL.swigValue(), "Can't write JPEG with an alpha channel");
                }

                if (renderParams.m_depth == 16)
                {
                    throw new MakoException(EDLErrorCode.JM_ERR_GENERAL.swigValue(), "Can't write JPEG with 16 bit depth");
                }

                // JPEG - middling quality
                pairImageFrame = IDOMJPEGImage.createWriterAndImage(jawsMako,
                                                                    renderParams.m_finalSpace,
                                                                    renderSpec.getWidth(), renderSpec.getHeight(),
                                                                    (short) 8,
                                                                    renderParams.m_xResolution,
                                                                    renderParams.m_yResolution,
                                                                    (short) 3,
                                                                    readStream, writeStream);
            }

            IImageFrameWriter   frame = null;
            IDOMImage           image = null;

            if (pairImageFrame != null)
            {
                frame = pairImageFrame.getFrameWriter();
                image = pairImageFrame.getDomImage();
            }
            
            if (frame != null)
            {
                // Out with it
                int ofs = 0;
                for (int y = 0; y < renderSpec.getHeight(); y++, ofs += rowStride)
                {
                    frame.writeScanLineOfs(frameBuffer, ofs);
                }
                frame.flushData();
            }
            frameBuffer = null;
        }
        catch (MakoException e)
        {
            errored = true;
            errorString = e.getErrorMsg();
        }
        catch (Exception e)
        {
            errored = true;
            errorString = e.getMessage();
        }
    }

    public IDOMFixedPage RenderToImage()
    {
        // Fetch the content
        fetchTimer.start ();
        IDOMFixedPage content = page.getContent();
        fetchTimer.end (1);

        // If we are being asked to perform overprint preview, find all the inks present on the page
        CEDLVectColorantInfo mergeSpots = new CEDLVectColorantInfo();
        if (renderParams.m_overprintPreview)
        {
            FindSpots.findAllSpots(jawsMako, content, renderParams.m_finalSpace, mergeSpots);
        }

        // Decide the size of the resulting rendered image. Here we'll just
        // render the crop box area.
        FRect cropBox       = page.getCropBox();
        int   pixelWidth    = (int) (cropBox.getDX() * renderParams.m_xResolution / 96.0 + 0.5);
        int   pixelHeight   = (int) (cropBox.getDY() * renderParams.m_yResolution / 96.0 + 0.5);

        // Populate the render spec
        CImageRenderSpec renderSpec = new CImageRenderSpec ();

        setCommonRenderParams (renderSpec, content, cropBox);
        
        // Render!
        renderPageTimer.start();
        renderer.render(content, renderSpec);
        renderPageTimer.end (1);

        IDOMColorSpace colorSpace = renderParams.m_finalSpace;
        IDOMImage image = renderSpec.getResult();

        // For 16 bit, encode now.
        if (renderParams.m_depth == 16)
        {
            // Use Mako's temp store to store the compressed data
            var readerWriter = jawsMako.getTempStore().createTemporaryReaderWriterPair();
            IRAInputStream tempReader = readerWriter.getInputStream();
            IRAOutputStream tempWriter = readerWriter.getOutputStream();

            // Create a flate compressor - medium compression
            IOutputStream flateWriter = IOutputStream.createToFlateCompressed(factory, tempWriter, 5, false);
            flateWriter.openE();

            // Compress.
            IImageFrame frame = image.getImageFrame(factory);
            byte[] scanline = new byte[(int) frame.getRawBytesPerRow()];
            int outputStride = renderParams.m_finalSpace.getNumComponents() * pixelWidth * 2;
            for (int y = 0; y < pixelHeight; y++)
            {
                frame.readScanLine(scanline);
                flateWriter.completeWriteE(scanline, (int)outputStride);
            }
            flateWriter.close();

            IInputStream tempInputStream = tempReader.toIInputStream();

            IDOMPDFImage.IDecodeParams decodeParams = IDOMPDFImage.FlateLZWParams.create();

            // Create the image
            image = IDOMPDFImage.create(factory,
                                        tempInputStream,
                                        eDOMImageType.eDITFlate,
                                        decodeParams,
                                        colorSpace, 
                                        pixelWidth, pixelHeight, 
                                        (short) 16,
                                        new CEDLVectFloat(), new CEDLVectUShort(),
                                        IDOMPDFImage.eImageAlpha.eIANone,
                                        true);
        }

        // Wrap that in a brush
        IDOMBrush brush = IDOMImageBrush.create(factory, image, new FRect(), cropBox);

        // Wrap in a path
        IDOMPathNode path = IDOMPathNode.createFilled(factory,
                                                      IDOMPathGeometry.create(factory, cropBox),
                                                      brush);

        // Create new content
        IDOMFixedPage newContent = IDOMFixedPage.create(factory, content.getWidth(), content.getHeight());
        newContent.setCropBox(cropBox);
        newContent.appendChild(path);

        page.revert();
        page.release();

        return newContent;
    }

    private IJawsMako           jawsMako;
    private IEDLClassFactory    factory;
    public  IApexRenderer       renderer;
    private ApexRenderParams    renderParams;

    private boolean             usetimer;

    private IOptionalContent    optionalContent;    // If the document uses Optional Content
    private IPage               page;               // The input page

    public int                  pageNumInFile;
    public boolean              errored;
    public String               errorString;
    
    public StopWatch            fetchTimer;
    public StopWatch            renderPageTimer;

    public int                  pixelStride;
    public int                  rowStride;
    public int                  frameBufSize; 
}
