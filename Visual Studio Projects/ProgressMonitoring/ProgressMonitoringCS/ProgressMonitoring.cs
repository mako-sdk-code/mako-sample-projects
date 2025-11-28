/* -----------------------------------------------------------------------
 * <copyright file="ProgressMonitoring.cs" company="Hybrid Software Helix Ltd">
 *  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Hybrid Software Helix Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 * </summary>
 * -----------------------------------------------------------------------
 */

using System;
using JawsMako;
using static JawsMako.jawsmakoIF_csharp;

namespace ProgressMonitoring
{
    class ConverterParams
    {
        public string InputFilePath { get; private set; }
        public eFileFormat InputFileFormat { get; private set; }

        public string OutputFilePath { get; private set; }
        public eFileFormat OutputFileFormat { get; private set; }

        public IProgressMonitor CustomTransformProgressMonitor { get; set; }
        public IProgressMonitor TransformProgressMonitor { get; set; }
        public IProgressMonitor TransformChainProgressMonitor { get; set; }

        public void SetInputPath(string path)
        {
            InputFilePath = path;
            InputFileFormat = FormatFromPath(path);
        }

        public void SetOutputPath(string path)
        {
            OutputFilePath = path;
            OutputFileFormat = FormatFromPath(path);
        }

        static eFileFormat FormatFromPath(string path)
        {
            string ext = System.IO.Path.GetExtension(path).ToLower();
            return ext switch
            {
                ".xps" => eFileFormat.eFFXPS,
                ".pdf" => eFileFormat.eFFPDF,
                ".svg" => eFileFormat.eFFSVG,
                ".ps" => eFileFormat.eFFPS,
                ".eps" => eFileFormat.eFFEPS,
                ".pcl" => eFileFormat.eFFPCL5,
                ".pxl" => eFileFormat.eFFPCLXL,
                ".ijp" => eFileFormat.eFFIJPDS,
                ".zip" => eFileFormat.eFFPPML,
                ".oxps" => eFileFormat.eFFOXPS,
                _ => throw new ArgumentException($"Unsupported file type: {path}")
            };
        }
    }

    class ProgressHandler : IProgressTickIntCallback
    {
        private readonly string info;
        private int tickCount;

        public IProgressTick ProgressTick { get; private set; }

        public ProgressHandler(string info)
        {
            this.info = info;
            tickCount = 0;
            ProgressTick = IProgressTick.create(this.getCallbackFunc(), this.getPriv());
        }

        public override void tick(uint currentCount, uint maxCount)
        {
            tickCount++;
            Console.WriteLine($"{info} : {tickCount}");
        }
    }

    class ProgressEventHandler : IProgressEventHandlerCallback
    {
        private int m_pageCount;
        private int m_nodeCount;
        private int m_nodeDepth;
        private readonly Stack<int> m_nodes = new Stack<int>();

        public ProgressEventHandler()
        {
            m_pageCount = 0;
            m_nodeCount = 0;
            m_nodeDepth = 0;
        }

        public override void handleEvent(IProgressEventHandler.Event evt)
        {
            int id;

            switch (evt)
            {
                case IProgressEventHandler.Event.eEvtPageWriteStart:
                    m_pageCount++;
                    m_nodeCount = 0;
                    m_nodeDepth = 0;
                    m_nodes.Clear();
                    Console.WriteLine($"start of page {m_pageCount}");
                    break;

                case IProgressEventHandler.Event.eEvtPageWriteEnd:
                    Console.WriteLine($"end of page {m_pageCount}, got {m_nodeCount} node events");
                    if (m_nodeDepth != 0)
                        Console.WriteLine($"mismatch in node write start/end {m_nodeDepth}");
                    break;

                case IProgressEventHandler.Event.eEvtNodeWriteStart:
                    id = ++m_nodeCount;
                    m_nodeDepth++;
                    m_nodes.Push(id);
                    if (id % 500 == 0)
                        Console.WriteLine($"start of node {id}");
                    break;

                case IProgressEventHandler.Event.eEvtNodeWriteEnd:
                    m_nodeDepth--;
                    if (m_nodes.Count == 0)
                    {
                        Console.WriteLine("mismatch, empty nodes");
                    }
                    else
                    {
                        id = m_nodes.Pop();
                        if (id % 500 == 0)
                            Console.WriteLine($"end of node {id}");
                    }
                    break;

                default:
                    break;
            }
        }

        public static ProgressEventHandler Create()
        {
            var handler = new ProgressEventHandler();
            handler.ProgressEvent = IProgressEventHandler.create(handler.getCallbackFunc(), handler.getPriv());
            return handler;
        }

        public IProgressEventHandler ProgressEvent { get; private set; }
    }

    class EmptyTransformImplementation : ICustomTransform.IImplementation
    {
        private IJawsMako jawsMako;
        public EmptyTransformImplementation(IJawsMako jm) => jawsMako = jm;
    }

    class ProgressMonitoring
    {
        static void OutputIterateByPage(ConverterParams cvtParams, IJawsMako jawsMako, IDocumentAssembly assembly, IOutput output)
        {
            var tempStore = jawsMako.getTempStore();
            var pair = tempStore.createTemporaryReaderWriterPair();
            IRAInputStream reader = pair.inputStream;
            IRAOutputStream writer = pair.outputStream;
            var writerHandle = output.openWriter(assembly, writer);

            // Create transforms
            var customImpl = new EmptyTransformImplementation(jawsMako);
            var customTransform = ICustomTransform.create(jawsMako, customImpl);
            customTransform.setProgressMonitor(cvtParams.CustomTransformProgressMonitor);

            var ccTransform = IColorConverterTransform.create(jawsMako);
            var deviceCmyk = IDOMColorSpaceDeviceCMYK.create(jawsMako);
            ccTransform.setTargetSpace(deviceCmyk);
            ccTransform.setProgressMonitor(cvtParams.TransformProgressMonitor);

            var transformChain = ITransformChain.create(jawsMako, cvtParams.TransformChainProgressMonitor);
            var colorConverter = IColorConverterTransform.create(jawsMako);
            transformChain.pushTransform(colorConverter);

            for (uint docNo = 0; assembly.documentExists(docNo); docNo++)
            {
                var document = assembly.getDocument(docNo);
                writerHandle.beginDocument(document);

                uint maxPages = 1;
                for (uint pageIndex = 0; pageIndex < maxPages; pageIndex++)
                {
                    var page = document.getPage(pageIndex);
                    var fixedPage = page.getContent();

                    // Progress monitor tests
                    customTransform.transformPage(page);

                    bool changed = false;
                    ccTransform.transform(fixedPage, ref changed);
                    transformChain.transform(fixedPage, ref changed);

                    writerHandle.writePage(page);
                    page.release();
                }

                writerHandle.endDocument();
            }

            writerHandle.finish();
            IOutputStream.copy(reader, IOutputStream.createToFile(jawsMako, cvtParams.OutputFilePath));
        }

        static void Usage(string progName) =>
            Console.WriteLine($"{progName} <input> <output>");

        static int Main(string[] args)
        {
            try
            {
                if (args.Length != 2)
                {
                    Usage(AppDomain.CurrentDomain.FriendlyName);
                    return 1;
                }

                var cvtParams = new ConverterParams();
                cvtParams.SetInputPath(args[0]);
                cvtParams.SetOutputPath(args[1]);

                var jawsMako = IJawsMako.create();
                IJawsMako.enableAllFeatures(jawsMako);

                var inputHandler = new ProgressHandler("input");
                var customHandler = new ProgressHandler("customtransform");
                var transformHandler = new ProgressHandler("transform");
                var chainHandler = new ProgressHandler("transformchain");

                // Use a ProgressEventHandler for the output to get more detailed information
                var outputHandler = ProgressEventHandler.Create();

                var abort = IAbort.create();
                cvtParams.CustomTransformProgressMonitor = IProgressMonitor.create(customHandler.ProgressTick, abort);
                cvtParams.TransformProgressMonitor = IProgressMonitor.create(transformHandler.ProgressTick, abort);
                cvtParams.TransformChainProgressMonitor = IProgressMonitor.create(chainHandler.ProgressTick, abort);

                var inputProgressMonitor = IProgressMonitor.create(inputHandler.ProgressTick, abort);
                var input = IInput.create(jawsMako, cvtParams.InputFileFormat, inputProgressMonitor);

                // Set ProgressEventHandler for output progress monitor
                var outputProgressMonitor = IProgressMonitor.create(abort);
                outputProgressMonitor.setProgressEventHandler(outputHandler.ProgressEvent);
                var output = IOutput.create(jawsMako, cvtParams.OutputFileFormat, outputProgressMonitor);
                
                var assembly = input.open(cvtParams.InputFilePath);

                OutputIterateByPage(cvtParams, jawsMako, assembly, output);
            }
            catch (MakoException e)
            {
                Console.WriteLine($"MakoException: {e.m_msg}");
                return (int)e.m_errorCode;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Exception: {ex.Message}");
                return 1;
            }
            return 0;
        }
    }
}
