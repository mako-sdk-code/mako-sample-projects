/* -----------------------------------------------------------------------
 * <copyright file="ProgressMonitoring.cpp" company="Global Graphics Software Ltd">
 *  Copyright (c) 2025 Global Graphics Software Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Global Graphics Software Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 * </summary>
 * -----------------------------------------------------------------------
 */

#include <iostream>
#include <memory>

#include <jawsmako/jawsmako.h>
#include <jawsmako/pdfinput.h>
#include <jawsmako/customtransform.h>
#include <deque>

using namespace JawsMako;
using namespace EDL;

class ConverterParams
{
public:
    std::wstring    m_inputFilePath;
    eFileFormat     m_inputFileFormat;

    std::wstring    m_outputFilePath;
    eFileFormat     m_outputFileFormat;

    IProgressMonitorPtr m_customTransformProgressMonitor;
    IProgressMonitorPtr m_transformProgressMonitor;
    IProgressMonitorPtr m_transformChainProgressMonitor;

    void setInputPath(const std::wstring& filePath)
    {
        m_inputFilePath = filePath;
        m_inputFileFormat = formatFromPath(filePath);
    }

    void setOutputPath(const std::wstring& filePath)
    {
        m_outputFilePath = filePath;
        m_outputFileFormat = formatFromPath(filePath);
    }

    // Determine the associated format for a given path from the file extension
    static eFileFormat formatFromPath(const std::wstring& path)
    {
        // Get the extension in lower case
        if (path.size() < 3)
        {
            // Cannot determine the extension if there isn't one!
            std::string message("Cannot determine file extension for path ");
            throw std::length_error(message);
        }

        size_t extensionPosition = path.find_last_of('.');
        if (extensionPosition != String::npos)
        {
            std::wstring extension = path.substr(extensionPosition);
            std::transform(extension.begin(), extension.end(), extension.begin(), towlower);

            if (extension == L".xps")
                return eFFXPS;
            if (extension == L".pdf")
                return eFFPDF;
            if (extension == L".svg")
                return eFFSVG;
            if (extension == L".ps")
                return eFFPS;
            if (extension == L".eps")
                return eFFEPS;
            if (extension == L".pcl")
                return eFFPCL5;
            if (extension == L".pxl")
                return eFFPCLXL;
            if (extension == L".ijp")
                return eFFIJPDS;
            if (extension == L".zip")
                return eFFPPML;
            if (extension == L".oxps")
                return eFFOXPS;
        }

        std::string message("Unsupported file type for path ");
        throw std::invalid_argument(message);
    }
};

class ProgressHandler
{
    // This section should be customised
public:

    explicit ProgressHandler(std::string info = "") : m_info(std::move(info)), m_tickCount(0) {}

    void tick(uint32_t currentCount, uint32_t maxCount)
    {
        m_tickCount++;

        printf("%s : %d\n", m_info.c_str(), m_tickCount);
        fflush(stdout);
    }

    // Everything below this part can be reused
    IProgressTickPtr    m_progressTick;

    static std::shared_ptr<ProgressHandler>  create(const std::string& info)
    {
        std::shared_ptr<ProgressHandler> progressHandler = std::make_shared<ProgressHandler>(info);

        progressHandler->m_progressTick = IProgressTick::create(callbackInt, progressHandler.get());

        return progressHandler;
    }

protected:

    static void callbackInt(void* priv, uint32_t currentCount, uint32_t maxCount)
    {
        ProgressHandler* progressHandler = (ProgressHandler*)priv;

        progressHandler->tick(currentCount, maxCount);
    }

private:

    std::string     m_info;
    int             m_tickCount;
};

class ProgressEventHandler {
public:
    // This section should be customised
    ProgressEventHandler() : m_pageCount(0), m_nodeCount(0), m_nodeDepth(0) {}
    void handleEvent(IProgressEventHandler::Event evt) {
        int id;
        switch (evt) {
        case IProgressEventHandler::eEvtPageWriteStart:
            m_pageCount++;
            m_nodeCount = 0;
            m_nodeDepth = 0;
            m_nodes.clear();
            printf("start of page %d\n", m_pageCount);
            break;
        case IProgressEventHandler::eEvtPageWriteEnd:
            printf("end of page %d, got %d node events\n", m_pageCount, m_nodeCount);
            if (m_nodeDepth != 0) printf("mismatch in node write start/end %d\n", m_nodeDepth);
            break;
        case IProgressEventHandler::eEvtNodeWriteStart:
            id = ++m_nodeCount;
            m_nodeDepth++;
            m_nodes.push_back(id);
            if (id % 500 == 0) {
                printf("start of node %d\n", id);
            }
            break;
        case IProgressEventHandler::eEvtNodeWriteEnd:
            m_nodeDepth--;
            if (m_nodes.empty())
                printf("mismatch, empty nodes\n");
            else {
                id = m_nodes.back();
                m_nodes.pop_back();
                if (id % 500 == 0) {
                    printf("end of node %d\n", id);
                }
            }
            break;
        default:
            break;
        }
    }
    // Everything below this part can be reused
    static std::shared_ptr<ProgressEventHandler> create() {
        std::shared_ptr<ProgressEventHandler> progressEventHandler = std::make_shared<ProgressEventHandler>();
        progressEventHandler->m_progressEventHandler = IProgressEventHandler::create(callback, progressEventHandler.get());
        return progressEventHandler;
    }
    int m_pageCount;
    int m_nodeCount;
    int m_nodeDepth;
    std::deque<int> m_nodes;
    IProgressEventHandlerPtr m_progressEventHandler;
protected:
    static void callback(void* priv, IProgressEventHandler::Event evt) {
        ProgressEventHandler* progressEventHandler = (ProgressEventHandler*)priv;
        if (progressEventHandler)
            progressEventHandler->handleEvent(evt);
    }
};

class CEmptyTransformImplementation : public ICustomTransform::IImplementation
{
public:
    CEmptyTransformImplementation(const IJawsMakoPtr& jawsMako)
    {
        m_jawsMako = jawsMako;
    }

protected:
    IJawsMakoPtr m_jawsMako;
};


static void output_iterateByPage(const ConverterParams& cvtParams,
    const IJawsMakoPtr& jawsMako,
    const IDocumentAssemblyPtr& assembly,
    const IOutputPtr& output)
{
    IRAInputStreamPtr reader;
    IRAOutputStreamPtr writer;
    jawsMako->getTempStore()->createTemporaryReaderWriterPair(reader, writer);

    const IOutputWriterPtr outputWriter = output->openWriter(assembly, writer);

    // Create the transform
    CEmptyTransformImplementation implementation(jawsMako);
    ITransformPtr customTransform = ICustomTransform::create(jawsMako, &implementation);
    customTransform->setProgressMonitor(cvtParams.m_customTransformProgressMonitor);

    IColorConverterTransformPtr ccTransform = IColorConverterTransform::create(jawsMako);
    IDOMColorSpacePtr deviceCmyk = IDOMColorSpaceDeviceCMYK::create(jawsMako);
    ccTransform->setTargetSpace(deviceCmyk);
    ccTransform->setProgressMonitor(cvtParams.m_transformProgressMonitor);


    ITransformChainPtr transformChain;
    {
        IColorConverterTransformPtr colorConverter = IColorConverterTransform::create(jawsMako);

        transformChain = ITransformChain::create(jawsMako, cvtParams.m_transformChainProgressMonitor);

        transformChain->pushTransform(colorConverter);
    }


    for (int docNo = 0; ; docNo++)
    {
        if (!assembly->documentExists(docNo))
        {
            // No more documents exist
            break;
        }

        IDocumentPtr document = assembly->getDocument(docNo);

        outputWriter->beginDocument(document);

        int pageStart = 0;

        constexpr int pageEnd = 1; // process one page only

        for (int pageIndex = pageStart; pageIndex < pageEnd; pageIndex++)
        {
            if (!document->pageExists(pageIndex))
            {
                // No more pages in this document.
                break;
            }

            IPagePtr page = document->getPage(pageIndex);

            // Test progress monitor in ICustomTransform
            {
                customTransform->transformPage(page);
            }

            // Test progress monitor in a single ITransform
            {
                IDOMFixedPagePtr fixedPage = page->getContent();
                bool changed = false;
                ccTransform->transform(fixedPage, changed);
            }

            // Test progress monitor in a transform chain
            {
                IDOMFixedPagePtr fixedPage = page->getContent();
                bool changed = false;
                transformChain->transform(fixedPage, changed);
            }

            outputWriter->writePage(page);

            page->release();
        }

        outputWriter->endDocument();
    }

    outputWriter->finish();

    IOutputStream::copy(reader, IOutputStream::createToFile(jawsMako, cvtParams.m_outputFilePath.c_str()));
}

static void usage(const std::wstring& progName)
{
    wprintf(L"%s <input> <output>", progName.c_str());
    fflush(stdout);
}


#ifdef _WIN32
int wmain(int argc, wchar_t* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    try
    {
        ConverterParams cvtParams;
        std::wstring    progName = argv[0];

        if (argc != 3)
        {
            usage(progName);
            return 1;
        }

        cvtParams.setInputPath(argv[1]);
        cvtParams.setOutputPath(argv[2]);

        // Create our JawsMako instance.
        const IJawsMakoPtr jawsMako = IJawsMako::create();
        IJawsMako::enableAllFeatures(jawsMako);

        std::shared_ptr<ProgressHandler>    inputProgressHandler = ProgressHandler::create("input");
        std::shared_ptr<ProgressHandler>    customTransformProgressHandler = ProgressHandler::create("customtransform");
        std::shared_ptr<ProgressHandler>    transformProgressHandler = ProgressHandler::create("transform");
        std::shared_ptr<ProgressHandler>    transformChainProgressHandler = ProgressHandler::create("transformChain");

        // Use a ProgressEventHandler for the output to get more detailed information
        std::shared_ptr<ProgressEventHandler>    outputProgressHandler = ProgressEventHandler::create();

        IAbortPtr           abort = IAbort::create();
        IProgressMonitorPtr inputProgressMonitor = IProgressMonitor::create(inputProgressHandler->m_progressTick, abort);
        
        cvtParams.m_customTransformProgressMonitor = IProgressMonitor::create(customTransformProgressHandler->m_progressTick, abort);
        cvtParams.m_transformProgressMonitor = IProgressMonitor::create(transformProgressHandler->m_progressTick, abort);
        cvtParams.m_transformChainProgressMonitor = IProgressMonitor::create(transformChainProgressHandler->m_progressTick, abort);

        // Set ProgressEventHandler for output progress monitor
        IProgressMonitorPtr outputProgressMonitor = IProgressMonitor::create(abort);
        outputProgressMonitor->setProgressEventHandler(outputProgressHandler->m_progressEventHandler);

        // Create input pointer
        IInputPtr input = IInput::create(jawsMako, cvtParams.m_inputFileFormat, inputProgressMonitor);

        // Create output pointer and set optional parameters
        const IOutputPtr output = IOutput::create(jawsMako, cvtParams.m_outputFileFormat, outputProgressMonitor);

        // Get the assembly from the input.
        const IDocumentAssemblyPtr assembly = input->open(cvtParams.m_inputFilePath.c_str());

        output_iterateByPage(cvtParams, jawsMako, assembly, output);
    }
    catch (IError& e)
    {
        String errorFormatString = getEDLErrorString(e.getErrorCode());
        std::wcerr << L"Exception thrown: " << e.getErrorDescription(errorFormatString) << std::endl;
#ifdef _WIN32
        // On windows, the return code allows larger numbers, and we can return the error code
        return static_cast<int>(e.getErrorCode());
#else
        // On other platforms, the exit code is masked to the low 8 bits. So here we just return
        // a fixed value.
        return 1;
#endif
    }
    catch (std::exception& e)
    {
        std::wcerr << L"std::exception thrown: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
