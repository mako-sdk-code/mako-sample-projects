/* -----------------------------------------------------------------------
 * <copyright file="OptionalContentSearch.java" company="Hybrid Software Helix Ltd">
 *  Copyright (C) 2025 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Hybrid Software Helix Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 * </summary>
 * -----------------------------------------------------------------------
 */

import java.util.*;
import com.globalgraphics.JawsMako.jawsmakoIF.*;

public class OptionalContentSearch
{
    public static void main(String[] args)
    {
        try
        {
            IJawsMako mako = IJawsMako.create();
            IJawsMako.enableAllFeatures(mako);

            // Load the input PDF and get its document
            IDocumentAssembly assembly = IPDFInput.create(mako).open(args[0]);
            IDocument document = assembly.getDocument();

            // Input is a multi-page .pdf and on each page, we have different optional content (Layer)
            IOptionalContent optionalContent = document.getOptionalContent();

            // Create a custom transform to do our searching
            COptionalContentSearchImplementation searchImpl = new COptionalContentSearchImplementation();
            ICustomTransform optionalContentSearchTransform =
                    ICustomTransform.create(mako, searchImpl);

            for (int pageIndex = 0; pageIndex < document.getNumPages(); pageIndex++)
            {
                System.out.printf("Page %d:\n", pageIndex + 1);

                // Clear state and force transform to check all shared resources
                searchImpl.reset();
                optionalContentSearchTransform.flushCaches();

                // Operate on a clone of the page to avoid DOM modification
                IPage clonedPage = document.getPage(pageIndex).clone();
                optionalContentSearchTransform.transformPage(clonedPage);

                // Output discovered optional content groups
                List<IOptionalContentGroupReference> foundGroups = searchImpl.getFoundGroups();
                for (IOptionalContentGroupReference foundGroup : foundGroups)
                {
                    IOptionalContentGroup group = optionalContent.getGroup(foundGroup);
                    System.out.printf("  Found group: %s\n", group.getName());
                }
            }
        }
        catch (Exception e)
        {
            System.out.println("Exception thrown: " + e);
        }
    }

    /**
     * Custom transform to find relevant optional content groups
     */
    static class COptionalContentSearchImplementation extends ICustomTransform.IImplementation
    {
        private final List<IOptionalContentGroupReference> foundGroups;

        public COptionalContentSearchImplementation()
        {
            foundGroups = new ArrayList<>();
        }

        public void reset()
        {
            foundGroups.clear();
        }

        public List<IOptionalContentGroupReference> getFoundGroups()
        {
            return foundGroups;
        }

        @Override
        public IDOMNode transformGroup(ICustomTransform.IImplementation genericImpl,
                                       IDOMGroup group,
                                       boolean[] changed,
                                       boolean transformChildren,
                                       CTransformState state)
        {
            IOptionalContentDetails details = group.getOptionalContentDetails();
            if (details != null)
            {
                CEDLVectIOptionalContentGroupReference refs = details.getGroupReferences();
                IOptionalContentGroupReference[] referencedGroups = refs.toArray();

                for (IOptionalContentGroupReference ref : referencedGroups)
                {
                    boolean alreadyFound = false;
                    for (IOptionalContentGroupReference existing : foundGroups)
                    {
                        if (existing.equals(ref))
                        {
                            alreadyFound = true;
                            break;
                        }
                    }

                    if (!alreadyFound)
                    {
                        foundGroups.add(ref);
                    }
                }
            }

            // Continue traversal into child groups
            return genericImpl.transformGroup(null, group, changed, transformChildren, state);
        }
    }
}
