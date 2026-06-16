#
# -----------------------------------------------------------------------
# <copyright file="reorder_layers.py" company="Hybrid Software Helix Ltd">
#  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
# </copyright>
# <summary>
#  Example demonstrating how to list and reorder PDF layers (Optional Content Groups)
#  using IOptionalContentConfiguration.setOrder().
# </summary>
# -----------------------------------------------------------------------
#
import sys

from jawsmakoIF_python import *

def reorder_layers(optional_content):
    """
    Move the bottommost layer to the top using IOptionalContentConfiguration.setOrder().
    """
    config = optional_content.getDefaultConfiguration()
    order = config.getOrder()

    if order.size() <= 1:
        print("Only one or no layers found. No reordering performed.")
        return

    bottom_layer = order[order.size() - 1]

    # Remove from end and insert at start
    order.erase(order.size() - 1)
    order.insert(0, bottom_layer)

    # Apply the new order to the configuration
    config.setOrder(order)
    optional_content.setDefaultConfiguration(config)

    print(f"Moved bottom layer to the top of z-order.\n")


def main():
    if len(sys.argv) != 3:
        print(f"Usage: python {sys.argv[0]} <input.pdf> <output.pdf>", file=sys.stderr)
        return 1

    try:
        jaws_mako = IJawsMako.create("")
        IJawsMako.enableAllFeatures(jaws_mako)

        input_pdf = sys.argv[1]
        output_pdf = sys.argv[2]

        # Open PDF
        pdf_input = IPDFInput.create(jaws_mako)
        assembly = pdf_input.open(input_pdf)
        document = assembly.getDocument()

        # Access the optional content (layers)
        optional_content = document.getOptionalContent()
        if optional_content is None:
            print("This PDF has no layers (optional content groups).")
            return 0

        # Reorder bottom layer to top
        reorder_layers(optional_content)

        # Save the reordered PDF
        pdf_output = IPDFOutput.create(jaws_mako)
        pdf_output.writeAssembly(assembly, output_pdf)

        print(f"Saved reordered PDF as: {output_pdf}")
        return 0
    except MakoException as e:
        print(f"MakoException: {e.m_msg}", file=sys.stderr)
        return 1
    except Exception as e:
        print(f"Exception thrown: {e}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
