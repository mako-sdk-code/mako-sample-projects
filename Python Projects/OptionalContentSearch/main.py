#
# -----------------------------------------------------------------------
# <copyright file="optional_content_search.py" company="Hybrid Software Helix Ltd">
#  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
# </copyright>
# <summary>
#  This example is provided on an "as is" basis and without warranty of any kind.
#  Hybrid Software Helix Ltd. does not warrant or make any representations
#  regarding the use or results of use of this example.
# </summary>
# -----------------------------------------------------------------------
#

import sys
from jawsmakoIF_python import *

class OptionalContentSearchImplementation(ICustomTransform.IImplementation):
    """Custom transform to find relevant optional content groups"""

    def __init__(self):
        super().__init__()
        self.m_foundGroups = []

    def reset(self):
        self.m_foundGroups.clear()

    def getFoundGroups(self):
        return self.m_foundGroups

    def transformGroup(self, genericImplementation, group, changed_ref, transformChildren, state):
        """Override transformGroup to collect optional content references"""
        details = group.getOptionalContentDetails()
        if details is not None:
            referencedGroups = details.getGroupReferences().toVector()
            for referencedGroup in referencedGroups:
                if not any(found.equals(referencedGroup) for found in self.m_foundGroups):
                    self.m_foundGroups.append(referencedGroup)
        # Continue traversal
        return genericImplementation.transformGroup(None, group, changed_ref, transformChildren, state)


def main():
    try:
        if len(sys.argv) < 2:
            print("Usage: python optional_content_search.py <input.pdf>")
            return 1

        input_path = sys.argv[1]

        # Instantiate Mako
        mako = IJawsMako.create("")
        IJawsMako.enableAllFeatures(mako)

        # Open the PDF document
        document = IPDFInput.create(mako).open(input_path).getDocument()

        # Get the optional content manager (layers)
        optionalContent = document.getOptionalContent()

        # Create our custom transform
        searchImpl = OptionalContentSearchImplementation()
        searchTransform = ICustomTransform.create(mako, searchImpl)

        # Iterate through each page
        for pageIndex in range(document.getNumPages()):
            print(f"Page {pageIndex + 1}:")

            # Reset and clear caches before processing a new page
            searchImpl.reset()
            searchTransform.flushCaches()

            # Operate on a clone to avoid modifying the DOM
            clonedPage = document.getPage(pageIndex).clone()
            searchTransform.transformPage(clonedPage)

            # Report found groups
            foundGroups = searchImpl.getFoundGroups()
            for foundGroup in foundGroups:
                group = optionalContent.getGroup(foundGroup)
                print(f"  Found group: {group.getName()}")

    except MakoException as e:
        print("Exception thrown:", e.m_msg)
        return 1
    except Exception as e:
        print(f"Exception thrown: {e}")
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
