/* --------------------------------------------------------------------------------
 *  <copyright file="Program.cs" company="Hybrid Software Helix Ltd">
 *    Copyright (c) 2025 Hybrid Software Helix Ltd. All rights reserved.
 *  </copyright>
 *  <summary>
 *    This example is provided on an "as is" basis and without warranty of any kind.
 *    Hybrid Software Helix Ltd. does not warrant or make any representations
 *    regarding the use or results of use of this example.
 *  </summary>
 * ---------------------------------------------------------------------------------
 */

using JawsMako;

namespace OptionalContentSearch
{
    internal class OptionalContentSearch
    {
        static int Main(string[] args)
        {
            try
            {
                var testFilepath = @"..\..\..\..\TestFiles\";

                var mako = IJawsMako.create();
                using var document = IPDFInput.create(mako).open(testFilepath + args[0]).getDocument();

                // Input is a multi-page .pdf and on each page, we have different optional content (Layer).
                var optionalContent = document.getOptionalContent();

                // Create a custom transform to do our searching
                var optionalContentSearchImplementation = new COptionalContentSearchImplementation();
                var optionalContentSearchTransform = ICustomTransform.create(mako, optionalContentSearchImplementation);

                for (uint pageIndex = 0; pageIndex < document.getNumPages(); pageIndex++)
                {
                    Console.WriteLine($"Page {pageIndex + 1}:");

                    // How we can know if the optionalContent is from this page?
                    // Ask the transform. Note that here we want to clear caches between checks as we always
                    // want the transform to descend into shared resources for each page. Caching would ordinarily
                    // preclude that.
                    //
                    // We operate on a clone to ensure no changes to the tree. Custom transforms automatically
                    // clean up duplicated resources which could cause an edit.
                    optionalContentSearchImplementation.reset();
                    optionalContentSearchTransform.flushCaches();
                    optionalContentSearchTransform.transformPage(document.getPage(pageIndex).clone());

                    // So what groups do we have?
                    var foundGroups = optionalContentSearchImplementation.getFoundGroups();
                    foreach (var foundGroup in foundGroups)
                    {
                        IOptionalContentGroup group = optionalContent.getGroup(foundGroup);

                        // This group is present - do something with this information
                        Console.WriteLine($"  Found group: {group.getName()}");
                    }
                }
            }
            catch (MakoException e)
            {
                Console.WriteLine("Exception thrown: " + e.m_msg);
                return 1;
            }
            catch (Exception e)
            {
                Console.WriteLine($"Exception thrown: {e}");
            }

            return 0;
        }

        /// <summary>
        /// Custom transform to find relevant optional content
        /// </summary>
        class COptionalContentSearchImplementation : ICustomTransform.IImplementation
        {
            public COptionalContentSearchImplementation()
            {
                m_foundGroups = new List<IOptionalContentGroupReference>();
            }

            public void reset()
            {
                m_foundGroups.Clear();
            }

            public List<IOptionalContentGroupReference> getFoundGroups()
            {
                return m_foundGroups;
            }

            public override IDOMNode transformGroup(ICustomTransform.IImplementation genericImplementation,
                IDOMGroup group,
                ref bool changed, bool transformChildren, CTransformState state)
            {
                // Does this group have optional content information?
                IOptionalContentDetails details = group.getOptionalContentDetails();
                if (details != null)
                {
                    // What groups does this reference?
                    var referencedGroups = details.getGroupReferences().toVector();

                    // Unfortunately for now we need to laboriously check to see if we've seen this before
                    foreach (IOptionalContentGroupReference referencedGroup in referencedGroups)
                    {
                        bool found = false;
                        foreach (IOptionalContentGroupReference foundGroup in m_foundGroups)
                        {
                            if (foundGroup.Equals(referencedGroup))
                            {
                                found = true;
                                break;
                            }
                        }

                        if (!found)
                        {
                            m_foundGroups.Add(referencedGroup);
                        }
                    }
                }

                // Descend further
                return genericImplementation.transformGroup(null, group, ref changed, transformChildren, state);
            }

            private readonly List<IOptionalContentGroupReference> m_foundGroups;
        }
    }
}