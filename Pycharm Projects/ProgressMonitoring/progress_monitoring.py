# -----------------------------------------------------------------------
# <copyright file="ProgressMonitoring.py" company="Hybrid Software Helix Ltd">
#  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
# </copyright>
# <summary>
#  This example is provided on an "as is" basis and without warranty of any kind.
#  Hybrid Software Helix Ltd. does not warrant or make any representations
#  regarding the use or results of use of this example.
# </summary>
# -----------------------------------------------------------------------

import sys
from jawsmakoIF_python import *


class ConverterParams:
    def __init__(self):
        self.input_file_path = None
        self.input_file_format = None
        self.output_file_path = None
        self.output_file_format = None
        self.custom_transform_progress_monitor = None
        self.transform_progress_monitor = None
        self.transform_chain_progress_monitor = None

    def set_input_path(self, path):
        self.input_file_path = path
        self.input_file_format = self.format_from_path(path)

    def set_output_path(self, path):
        self.output_file_path = path
        self.output_file_format = self.format_from_path(path)

    @staticmethod
    def format_from_path(path):
        ext = path.lower().split('.')[-1]
        mapping = {
            "xps": eFFXPS,
            "pdf": eFFPDF,
            "svg": eFFSVG,
            "ps": eFFPS,
            "eps": eFFEPS,
            "pcl": eFFPCL5,
            "pxl": eFFPCLXL,
            "ijp": eFFIJPDS,
            "zip": eFFPPML,
            "oxps": eFFOXPS
        }
        if ext in mapping:
            return mapping[ext]
        raise ValueError(f"Unsupported file type: {path}")


class ProgressHandler(IProgressTickIntCallback):
    def __init__(self, info):
        super().__init__()
        self.info = info
        self.tick_count = 0
        self.progress_tick = IProgressTick.create(self.getCallbackFunc(), self.getPriv())

    def tick(self, current_count, max_count):
        self.tick_count += 1
        print(f"{self.info} : {self.tick_count}")


class ProgressEventHandler:
    class Callback(IProgressEventHandlerCallback):
        def __init__(self):
            super().__init__()
            self.m_pageCount = 0
            self.m_nodeCount = 0
            self.m_nodeDepth = 0
            self.m_nodes = []

        def handleEvent(self, evt):
            if evt == IProgressEventHandler.eEvtPageWriteStart:
                self.m_pageCount += 1
                self.m_nodeCount = 0
                self.m_nodeDepth = 0
                self.m_nodes.clear()
                print(f"start of page {self.m_pageCount}")

            elif evt == IProgressEventHandler.eEvtPageWriteEnd:
                print(f"end of page {self.m_pageCount}, got {self.m_nodeCount} node events")
                if self.m_nodeDepth != 0:
                    print(f"mismatch in node write start/end {self.m_nodeDepth}")

            elif evt == IProgressEventHandler.eEvtNodeWriteStart:
                self.m_nodeCount += 1
                self.m_nodeDepth += 1
                self.m_nodes.append(self.m_nodeCount)
                if self.m_nodeCount % 500 == 0:
                    print(f"start of node {self.m_nodeCount}")

            elif evt == IProgressEventHandler.eEvtNodeWriteEnd:
                self.m_nodeDepth -= 1
                if not self.m_nodes:
                    print("mismatch, empty nodes")
                else:
                    node_id = self.m_nodes.pop()
                    if node_id % 500 == 0:
                        print(f"end of node {node_id}")

    def __init__(self):
        super().__init__()
        self.callback = ProgressEventHandler.Callback()
        cbFunc = self.callback.getCallbackFunc()
        cbPriv = self.callback.getPriv()
        self.progressEventHandler = IProgressEventHandler.create(cbFunc, cbPriv)

class EmptyTransformImplementation(ICustomTransform.IImplementation):
    def __init__(self, jaws_mako):
        super().__init__()
        self.jaws_mako = jaws_mako


def output_iterate_by_page(cvt_params, jaws_mako, assembly, output):
    temp_store = jaws_mako.getTempStore()
    pair = temp_store.createTemporaryReaderWriterPair()
    reader = pair[0]
    writer = pair[1]
    writer_handle = output.openWriter(assembly, writer)

    # Create transforms
    custom_impl = EmptyTransformImplementation(jaws_mako)
    custom_transform = ICustomTransform.create(jaws_mako, custom_impl)
    custom_transform.setProgressMonitor(cvt_params.custom_transform_progress_monitor)

    cc_transform = IColorConverterTransform.create(jaws_mako)
    device_cmyk = IDOMColorSpaceDeviceCMYK.create(jaws_mako.getFactory())
    cc_transform.setTargetSpace(device_cmyk)
    cc_transform.setProgressMonitor(cvt_params.transform_progress_monitor)

    transform_chain = ITransformChain.create(jaws_mako, cvt_params.transform_chain_progress_monitor)
    color_converter = IColorConverterTransform.create(jaws_mako)
    transform_chain.pushTransform(color_converter)

    doc_no = 0
    while assembly.documentExists(doc_no):
        document = assembly.getDocument(doc_no)
        writer_handle.beginDocument(document)

        page_index = 0
        while page_index < 1:
            page = document.getPage(page_index)
            fixed_page = page.getContent()

            # Progress monitor tests
            custom_transform.transformPage(page)

            changed = False
            cc_transform.transform(fixed_page, changed)
            transform_chain.transform(fixed_page, changed)

            writer_handle.writePage(page)
            page.release()
            page_index += 1

        writer_handle.endDocument()
        doc_no += 1

    writer_handle.finish()
    IOutputStream.copy(reader, IOutputStream.createToFile(jaws_mako.getFactory(), cvt_params.output_file_path))


def usage(prog_name):
    print(f"{prog_name} <input> <output>")


def main():
    try:
        if len(sys.argv) != 3:
            usage(sys.argv[0])
            return 1

        cvt_params = ConverterParams()
        cvt_params.set_input_path(sys.argv[1])
        cvt_params.set_output_path(sys.argv[2])

        jaws_mako = IJawsMako.create("")
        IJawsMako.enableAllFeatures(jaws_mako)

        input_handler = ProgressHandler("input")
        custom_handler = ProgressHandler("customtransform")
        transform_handler = ProgressHandler("transform")
        chain_handler = ProgressHandler("transformchain")

        # Use a ProgressEventHandler for the output to get more detailed information
        output_handler = ProgressEventHandler()

        abort = IAbort.create()
        cvt_params.custom_transform_progress_monitor = IProgressMonitor.create(custom_handler.progress_tick, abort)
        cvt_params.transform_progress_monitor = IProgressMonitor.create(transform_handler.progress_tick, abort)
        cvt_params.transform_chain_progress_monitor = IProgressMonitor.create(chain_handler.progress_tick, abort)

        input = IInput.create(jaws_mako, cvt_params.input_file_format,
                              IProgressMonitor.create(input_handler.progress_tick, abort))

        # Set ProgressEventHandler for output progress monitor
        output_progress_monitor = IProgressMonitor.create(abort)
        output_progress_monitor.setProgressEventHandler(output_handler.progressEventHandler)
        output = IOutput.create(jaws_mako, cvt_params.output_file_format,
                                output_progress_monitor)

        assembly = input.open(cvt_params.input_file_path)

        output_iterate_by_page(cvt_params, jaws_mako, assembly, output)

    except MakoException as e:
        print(f"MakoException: {e.m_msg}")
        return int(e.m_errorCode)
    except Exception as ex:
        print(f"Exception: {ex}")
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
