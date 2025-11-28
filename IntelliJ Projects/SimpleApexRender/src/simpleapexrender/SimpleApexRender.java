/*
 * Copyright (C) 2024-2025 Global Graphics Software Ltd. All rights reserved.
 *
 *  Simple sample application to demonstrate very basic rendering using Apex.
 */

package simpleapexrender;

import java.util.*;
import java.util.function.*;
import java.nio.*;
import com.globalgraphics.JawsMako.jawsmakoIF.*;
import com.globalgraphics.JawsMako.jawsmakoIF.jawsmakoIF.*;
import simpleapexrender.WorkThreads.*;

/**
 *
 */
public class SimpleApexRender 
{

    public static void main(String[] args)
    {
        int retCode = 0;

        try
        {
            // Create our JawsMako instance.
            IJawsMako jawsMako = IJawsMako.create ();
            IEDLClassFactory factory = jawsMako.getFactory();
            
            IJawsMako.enableAllFeatures (jawsMako);

            // Map DeviceGray to CMYK
            IColorManager colorMgr = IColorManager.get (factory);
            colorMgr.setMapDeviceGrayToCMYKBlack (true);

            ApexRenderParams renderParams = new ApexRenderParams (jawsMako, "simpleapexrender");

            // Fetch command line parameters
            for (String paramStr : args)
            {
                renderParams.addCommandLineParameter (paramStr);
            }

            // And parse them
            if (! renderParams.parseCommandLineParameters ())
            {
                System.exit (1);
            }


            // Create our input and open the assembly.
            // We're making a single pass through the assembly, so set sequential mode.
            IInput input = IInput.create (jawsMako, renderParams.m_inputFileFormat);
            input.setSequentialMode(true);
            IDocumentAssembly assembly = input.open(renderParams.m_inputFilePath);

            // Determine the final output color space, which is the final space if provided,
            // or the process space if not.
            if (renderParams.m_finalSpace == null)
            {
                renderParams.m_finalSpace = renderParams.m_processSpace;
            }

            // If there is a CMYK output intent profile, and we've been directed to use it,
            // replace the process color space. Further, if the output space is DeviceCMYK,
            // replace that also. We'll also tell the renderer to ignore intercepts to ensure
            // that any DeviceCMYK content is treated as the intent space.
            if (renderParams.m_useOutputIntent)
            {
                // If the document has a CMYK output intent, find it.
                // We'll use the first item we can find.
                // We could be doing per-page intents here, but for this sample, document level will do.
                IDOMColorSpace cmykIntent = null;
                IOutputIntent[] outputIntents = assembly.getDocument().getOutputIntents().toArray();
                for (int i = 0; cmykIntent == null && i < outputIntents.length; i++)
                {
                    IDOMICCProfile intent = outputIntents[i].getProfile();
                    if (intent != null)
                    {
                        IDOMColorSpace intentSpace = IDOMColorSpaceICCBased.create(factory, intent);
                        if (intentSpace.getNumComponents() == 4)
                        {
                            // Assume this is the one we want
                            cmykIntent = intentSpace;
                        }
                    }
                }
                if (cmykIntent != null)
                {
                    renderParams.m_processSpace = cmykIntent;
                    if (renderParams.m_finalSpace.getColorSpaceType() == IDOMColorSpace.eColorSpaceType.eDeviceCMYK)
                    {
                        renderParams.m_finalSpace = cmykIntent;
                    }

                    // And ignore intercepts; we want this color space to be treated as DeviceCMYK
                    renderParams.m_ignoreIntercepts = true;
                }
            }
                
            // We have a separate code path if we're generating PDF output or bare images.
            if (renderParams.m_renderedFormat == eRenderedFormat.eRFPDF)
            {
                // Writing to a PDF

                // Create a PDF output
                IOutput output = IOutput.create(jawsMako, eFileFormat.eFFPDF);

                // Create a writer
                IOutputWriter writer = output.openWriter(assembly, renderParams.m_outputFilePath);

                // Create our renderer
                IApexRenderer renderer = IApexRenderer.create(jawsMako);
                
                StopWatch avgRenderTime = new StopWatch ("Render Avg");

                // Iterate through all the documents and pages
                int pageNumInFile = 1;
                for (int docNum = 0; assembly.documentExists(docNum); docNum++)
                {
                    IDocument           document        = assembly.getDocument(docNum);
                    IOptionalContent    optionalContent = document.getOptionalContent();

                    writer.beginDocument(document);

                    for (int pageNum = 0; document.pageExists(pageNum); pageNum++, pageNumInFile++)
                    {
                        IPage page = document.getPage(pageNum);

                        // Fetch the content
                        IDOMFixedPage       content         = page.getContent();

                        RenderPage workItem = new RenderPage(   renderer,
                                                                renderParams,
                                                                page,
                                                                optionalContent,
                                                                pageNumInFile);

                        IDOMFixedPage newContent = workItem.RenderToImage ();

                        if (workItem.errored)
                        {
                            System.out.printf("Error on page %d", workItem.pageNumInFile);
                            throw new MakoException(EDLErrorCode.JM_ERR_GENERAL.swigValue(), workItem.errorString);
                        }

                        workItem.fetchTimer.printTimeDiff();
                        workItem.renderPageTimer.printTimeDiff();

                        avgRenderTime.m_delta += workItem.renderPageTimer.m_delta;
                        avgRenderTime.m_totalCount++;
                        
                        // And write that in a page
                        IPage newPage = IPage.create(jawsMako);
                        newPage.setContent(newContent);
                        writer.writePage(newPage);
                    }

                    writer.endDocument();
                }

                avgRenderTime.printSummary();
                
                // Done
                writer.finish();
            }
            else if (!renderParams.m_useAllGpus && renderParams.m_threadsPerGpu == 1)
            {
                // Create our renderer
                IApexRenderer renderer = IApexRenderer.create(jawsMako);

                // Writing to a series of images. The output file path should be a template of the file name.
                // Here we'll use frame buffers as it's going to be more efficient than having the renderer produce a
                // stored image, and then re-encoding it here.

                StopWatch avgRenderTime = new StopWatch ("Render Avg");

                // Iterate through all the documents and pages
                int pageNumInFile = 1;
                for (int docNum = 0; assembly.documentExists(docNum); docNum++)
                {
                    IDocument           document        = assembly.getDocument(docNum);
                    IOptionalContent    optionalContent = document.getOptionalContent();

                    for (int pageNum = 0; document.pageExists(pageNum); pageNum++, pageNumInFile++)
                    {
                        IPage page = document.getPage(pageNum);

                        RenderPage renderItem = new RenderPage(renderer,
                                                     renderParams,
                                                     page,
                                                     optionalContent,
                                                     pageNumInFile);

                        renderItem.RenderToFrameBuffer ();

                        if (renderItem.errored)
                        {
                            System.out.printf("Error on page %d\n", renderItem.pageNumInFile);
                            throw new MakoException(EDLErrorCode.JM_ERR_GENERAL.swigValue(), renderItem.errorString);
                        }

                        renderItem.fetchTimer.printTimeDiff();
                        renderItem.renderPageTimer.printTimeDiff();

                        avgRenderTime.m_delta += renderItem.renderPageTimer.m_delta;
                        avgRenderTime.m_totalCount++;
                    }
                }

                avgRenderTime.printSummary();

            }            
            else
            {
                // Finally, we're either attempting to use multiple threads, multiple GPUs, or both

                // Create a render queue with a limit of 30 entries
                WorkQueue<RenderPage> renderQueue = new WorkQueue<RenderPage> (30);

                // Scan and work out what GPUs we want to use
                ArrayList<Integer> useGpuIndexes = new ArrayList<Integer>();
                if (renderParams.m_useAllGpus)
                {
                    IApexRenderer.CGpuDeviceInfo[] devices = IApexRenderer.enumerateGpus().toArray();
                    if (devices.length == 0)
                    {
                        throw new MakoException (EDLErrorCode.JM_ERR_GENERAL.swigValue(), "Found no Vulkan devices!");
                    }
                    // For some reason current Vulkan may report the same integrated graphics more than once, at least on Windows.
                    boolean seenIntegrated = false;

                    // Ok - choose
                    for (int i = 0; i < devices.length; i++)
                    {
                        if (!devices[i].getDiscreteDevice())
                        {
                            // If we have seen an integrated device already, ignore.
                            if (seenIntegrated)
                            {
                                continue;
                            }

                            seenIntegrated = true;

                            // And we may not be using integrated devices
                            if (renderParams.m_excludeIntegratedGpus)
                            {
                                continue;
                            }
                        }

                        System.out.printf ("Using GPU %s at index %d\n", devices[i].getDeviceName(), i);
                        useGpuIndexes.add ((int) i);
                    }
                }
                else
                {
                    useGpuIndexes.add (-1); // Use the default GPU
                }


                // Create the render threads

                Thread[] renderThreads = new Thread [useGpuIndexes.size() * renderParams.m_threadsPerGpu];


                int threadIndex = 0;
                for (int gpuIndex : useGpuIndexes)
                {
                    // Create the renderer
                    IApexRenderer renderer = IApexRenderer.create (jawsMako, gpuIndex);

                    Predicate<RenderPage> processItemCB = (RenderPage item) ->
                    {
                        // We need to use a different renderer
                        item.renderer = renderer;
                        item.RenderToFrameBuffer();
                        return true;
                    };

                    for (int gpuThread = 0; gpuThread < renderParams.m_threadsPerGpu; gpuThread++)
                    {
                        Worker<RenderPage> worker = new Worker<RenderPage> (renderQueue, processItemCB);

                        Thread thread = new Thread( () ->
                            {
                                worker.ProcessMessages ();
                            }
                        );
                        thread.start ();

                        renderThreads [threadIndex] = thread;
                        threadIndex++;
                    }
                }

                // Iterate through all the documents and pages and create items
                ArrayList<RenderPage> workItems = new ArrayList<RenderPage>();
                int pageNumInFile = 1;
                for (int docNum = 0; assembly.documentExists (docNum); docNum++)
                {
                    IDocument           document        = assembly.getDocument (docNum);
                    IOptionalContent    optionalContent = document.getOptionalContent ();

                    for (int pageNum = 0; document.pageExists (pageNum); pageNum++, pageNumInFile++)
                    {
                        IPage page = document.getPage(pageNum);

                        // Create the work item
                        RenderPage item = new RenderPage (null,
                                               renderParams,
                                               page,
                                               optionalContent,
                                               pageNumInFile);

                        // Note for later checking
                        workItems.add (item);

                        // And add to the render queue
                        renderQueue.Add(item);
                    }
                }

                // Let everyone know that no more items are coming
                renderQueue.setQueueFinished (true);

                // And then wait for them to exit
                for (Thread renderThread :  renderThreads)
                {
                    renderThread.join();
                }

                StopWatch avgRenderTime = new StopWatch ("Render Avg");
                
                // Now check for any errors
                for (RenderPage item : workItems)
                {
                    if (item.errored)
                    {
                        System.out.printf ("Error on page %d\n", item.pageNumInFile);
                        throw new MakoException (EDLErrorCode.JM_ERR_GENERAL.swigValue(), item.errorString);
                    }
                    
                    item.fetchTimer.printTimeDiff();
                    item.renderPageTimer.printTimeDiff();

                    avgRenderTime.m_delta += item.renderPageTimer.m_delta;
                    avgRenderTime.m_totalCount++;
                }

                avgRenderTime.printSummary();
                
                // Done
            }
        }
        catch (MakoException e)
        {
            EDLErrorCode errCode = EDLErrorCode.swigToEnum(e.getErrorCode ());
            String errorFormatString = IJawsMako.getEDLErrorString(errCode);
            System.out.printf ("MakoException thrown: %s\n}", errorFormatString);
            retCode = (int) e.getErrorCode();
        }
        catch (Exception e)
        {
            System.out.printf ("Exception thrown: %s\n", e.toString());
            retCode = 1;
        }
    
        System.exit (retCode);
    }
}
