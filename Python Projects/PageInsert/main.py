#
# -----------------------------------------------------------------------
# <copyright file="insert_cover_page.py" company="Hybrid Software Helix Ltd">
#  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
# </copyright>
# <summary>
#  Inserts a cover page into an existing PDF and measures processing time.
# </summary>
# -----------------------------------------------------------------------
#

import sys
import time
from jawsmakoIF_python import *

def main():
    if len(sys.argv) < 3:
        print("Usage: python insert_cover_page.py <input.pdf> <output.pdf> <cover.pdf>")
        return 1

    test_file_path = "TestFiles/"
    input_file = test_file_path + sys.argv[1]
    output_file = sys.argv[2]
    cover_file = test_file_path + sys.argv[3]


    try:
        # Start timer
        start_time = time.time()

        # Instantiate Mako
        jaws_mako = IJawsMako.create("")
        IJawsMako.enableAllFeatures(jaws_mako)
        factory = jaws_mako.getFactory()

        # File we want to insert into
        input_stream = IInputStream.createFromFile(factory, input_file)

        # File we want to get our cover page from
        insert_stream = IInputStream.createFromFile(factory, cover_file)

        # Create the page inserter
        page_inserter = IPDFPageInserter.create(jaws_mako, input_stream)

        # Do the insertion (page numbers are zero-based)
        page_inserter.insert(insert_stream, 0, 0, 1)

        # Save the result
        output_stream = IOutputStream.createToFile(factory, output_file)
        page_inserter.save(output_stream)

        # Stop timer
        elapsed = time.time() - start_time
        print(f"Processing time: {elapsed:.4f} seconds")

    except MakoException as e:
        print(f"MakoException: {e.m_msg}")
        return 1
    except Exception as e:
        print(f"Unexpected error: {e}")
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
