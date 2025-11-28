/* -----------------------------------------------------------------------
 * <copyright file="ProgressMonitoring.java" company="Hybrid Software Helix Ltd">
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
import com.globalgraphics.JawsMako.jawsmakoIF.jawsmakoIF.*;
import java.io.*;
import java.util.Stack;

public class ProgressMonitoring {

    static class ConverterParams {
        public String inputFilePath;
        public eFileFormat inputFileFormat;
        public String outputFilePath;
        public eFileFormat outputFileFormat;
        public IProgressMonitor customTransformProgressMonitor;
        public IProgressMonitor transformProgressMonitor;
        public IProgressMonitor transformChainProgressMonitor;

        public void setInputPath(String path) {
            inputFilePath = path;
            inputFileFormat = formatFromPath(path);
        }

        public void setOutputPath(String path) {
            outputFilePath = path;
            outputFileFormat = formatFromPath(path);
        }

        private static eFileFormat formatFromPath(String path) {
            String ext = path.substring(path.lastIndexOf('.')).toLowerCase();
            return switch (ext) {
                case ".xps" -> eFileFormat.eFFXPS;
                case ".pdf" -> eFileFormat.eFFPDF;
                case ".svg" -> eFileFormat.eFFSVG;
                case ".ps" -> eFileFormat.eFFPS;
                case ".eps" -> eFileFormat.eFFEPS;
                case ".pcl" -> eFileFormat.eFFPCL5;
                case ".pxl" -> eFileFormat.eFFPCLXL;
                case ".ijp" -> eFileFormat.eFFIJPDS;
                case ".zip" -> eFileFormat.eFFPPML;
                case ".oxps" -> eFileFormat.eFFOXPS;
                default -> throw new IllegalArgumentException("Unsupported file type: " + path);
            };
        }
    }

    static class ProgressHandler extends IProgressTickIntCallback
    {
        private final String info;
        private int tickCount;
        public IProgressTick progressTick;

        public ProgressHandler(String info) {
            this.info = info;
            this.tickCount = 0;

            // Create callback + tick object
            this.progressTick = IProgressTick.create(getCallbackFunc(), getPriv());
        }

        @Override
        public void tick(long currentCount, long maxCount) {
            tickCount++;
            System.out.println(info + " : " + tickCount);
        }
    }

    public static class ProgressEventHandler extends IProgressEventHandlerCallback
    {
        private int m_pageCount;
        private int m_nodeCount;
        private int m_nodeDepth;
        private final Stack<Integer> m_nodes = new Stack<>();

        private IProgressEventHandler progressEventHandler;

        public ProgressEventHandler()
        {
            m_pageCount = 0;
            m_nodeCount = 0;
            m_nodeDepth = 0;
        }

        @Override
        public void handleEvent(IProgressEventHandler.Event evt)
        {
            int id;

            switch (evt)
            {
                case eEvtPageWriteStart:
                    m_pageCount++;
                    m_nodeCount = 0;
                    m_nodeDepth = 0;
                    m_nodes.clear();
                    System.out.printf("start of page %d%n", m_pageCount);
                    break;

                case eEvtPageWriteEnd:
                    System.out.printf("end of page %d, got %d node events%n", m_pageCount, m_nodeCount);
                    if (m_nodeDepth != 0)
                        System.out.printf("mismatch in node write start/end %d%n", m_nodeDepth);
                    break;

                case eEvtNodeWriteStart:
                    id = ++m_nodeCount;
                    m_nodeDepth++;
                    m_nodes.push(id);
                    if (id % 500 == 0)
                        System.out.printf("start of node %d%n", id);
                    break;

                case eEvtNodeWriteEnd:
                    m_nodeDepth--;
                    if (m_nodes.isEmpty())
                    {
                        System.out.println("mismatch, empty nodes");
                    }
                    else
                    {
                        id = m_nodes.pop();
                        if (id % 500 == 0)
                            System.out.printf("end of node %d%n", id);
                    }
                    break;

                default:
                    break;
            }
        }

        public static ProgressEventHandler create()
        {
            ProgressEventHandler handler = new ProgressEventHandler();
            handler.progressEventHandler = IProgressEventHandler.create(
                    handler.getCallbackFunc(),
                    handler.getPriv());
            return handler;
        }

        public IProgressEventHandler getProgressEventHandler()
        {
            return progressEventHandler;
        }
    }
    static class EmptyTransformImplementation extends ICustomTransform.IImplementation {
        private IJawsMako jawsMako;
        public EmptyTransformImplementation(IJawsMako jm) { jawsMako = jm; }
    }

    static void outputIterateByPage(ConverterParams cvtParams, IJawsMako jawsMako, IDocumentAssembly assembly, IOutput output) {
        var tempStore = jawsMako.getTempStore();
        var pair = tempStore.createTemporaryReaderWriterPair();
        IRAInputStream reader = pair.getInputStream();
        IRAOutputStream writer = pair.getOutputStream();

        IOutputWriter writerHandle = output.openWriter(assembly, writer);

        // Create transforms
        var customImpl = new EmptyTransformImplementation(jawsMako);
        var customTransform = ICustomTransform.create(jawsMako, customImpl);
        customTransform.setProgressMonitor(cvtParams.customTransformProgressMonitor);

        var ccTransform = IColorConverterTransform.create(jawsMako);
        var deviceCmyk = IDOMColorSpaceDeviceCMYK.create(jawsMako.getFactory());
        ccTransform.setTargetSpace(deviceCmyk);
        ccTransform.setProgressMonitor(cvtParams.transformProgressMonitor);

        var transformChain = ITransformChain.create(jawsMako, cvtParams.transformChainProgressMonitor);
        var colorConverter = IColorConverterTransform.create(jawsMako);
        transformChain.pushTransform(colorConverter);

        for (int docNo = 0; assembly.documentExists(docNo); docNo++) {
            IDocument document = assembly.getDocument(docNo);
            writerHandle.beginDocument(document);

            // First page only
            for (int pageIndex = 0; pageIndex < 1; pageIndex++) {
                IPage page = document.getPage(pageIndex);
                IDOMFixedPage fixedPage = page.getContent();

                customTransform.transformPage(page);

                boolean[] changed = { false };
                ccTransform.transform(fixedPage, changed);
                transformChain.transform(fixedPage, changed);

                writerHandle.writePage(page);
                page.release();
            }

            writerHandle.endDocument();
        }

        writerHandle.finish();
        IOutputStream.copy(reader, IOutputStream.createToFile(jawsMako.getFactory(), cvtParams.outputFilePath));
    }

    static void usage(String progName) {
        System.out.println(progName + " <input> <output>");
    }

    public static void main(String[] args) {
        try {
            if (args.length != 2) {
                usage("ProgressMonitoring");
                System.exit(1);
            }

            ConverterParams cvtParams = new ConverterParams();
            cvtParams.setInputPath(args[0]);
            cvtParams.setOutputPath(args[1]);

            IJawsMako jawsMako = IJawsMako.create();
            IJawsMako.enableAllFeatures(jawsMako);

            ProgressHandler inputHandler = new ProgressHandler("input");
            ProgressHandler customHandler = new ProgressHandler("customtransform");
            ProgressHandler transformHandler = new ProgressHandler("transform");
            ProgressHandler chainHandler = new ProgressHandler("transformchain");

            // Use a ProgressEventHandler for the output to get more detailed information
            ProgressEventHandler outputHandler = ProgressEventHandler.create();

            IAbort abort = IAbort.create();
            cvtParams.customTransformProgressMonitor = IProgressMonitor.create(customHandler.progressTick, abort);
            cvtParams.transformProgressMonitor = IProgressMonitor.create(transformHandler.progressTick, abort);
            cvtParams.transformChainProgressMonitor = IProgressMonitor.create(chainHandler.progressTick, abort);

            IInput input = IInput.create(jawsMako, cvtParams.inputFileFormat, IProgressMonitor.create(inputHandler.progressTick, abort));

            // Set ProgressEventHandler for output progress monitor
            IProgressMonitor outputProgressMonitor = IProgressMonitor.create(abort);
            outputProgressMonitor.setProgressEventHandler(outputHandler.getProgressEventHandler());
            IOutput output = IOutput.create(jawsMako, cvtParams.outputFileFormat, outputProgressMonitor);
            IDocumentAssembly assembly = input.open(cvtParams.inputFilePath);

            outputIterateByPage(cvtParams, jawsMako, assembly, output);
        } catch (Exception ex) {
            System.out.println("Exception: " + ex.getMessage());
            System.exit(1);
        }
    }
}
