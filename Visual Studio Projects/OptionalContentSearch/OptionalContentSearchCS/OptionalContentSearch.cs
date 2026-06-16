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

namespace OptionalContentSearchCS;

internal class OptionalContentSearch
{
    private const string TestFilesPath = @"..\..\..\..\..\..\TestFiles\";

    static int Main(string[] args)
    {
        if (args.Length != 1)
        {
            Console.Error.WriteLine($"Usage: {AppDomain.CurrentDomain.FriendlyName} <input.pdf>");
            return 1;
        }

        try
        {

            using var mako = IJawsMako.create();
            IJawsMako.enableAllFeatures(mako);
            using var document = IPDFInput.create(mako).open(TestFilesPath + args[0]).getDocument();

            // Input is a multi-page .pdf and on each page, we have different optional content (Layer).
            var optionalContent = document.getOptionalContent();

            // Create a custom transform to do our searching
            using var optionalContentSearchImplementation = new COptionalContentSearchImplementation();
            using var optionalContentSearchTransform = ICustomTransform.create(mako, optionalContentSearchImplementation);

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
                optionalContentSearchImplementation.Reset();
                optionalContentSearchTransform.flushCaches();
                optionalContentSearchTransform.transformPage(document.getPage(pageIndex).clone());

                // So what groups do we have?
                var foundGroups = optionalContentSearchImplementation.GetFoundGroups();
                foreach (var group in foundGroups.Select(foundGroup => optionalContent.getGroup(foundGroup)))
                {
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
            return 1;
        }

        return 0;
    }

    /// <summary>
    /// Custom transform to find relevant optional content
    /// </summary>
    private class COptionalContentSearchImplementation : ICustomTransform.IImplementation
    {
        public void Reset() => m_foundGroups.Clear();

        public List<IOptionalContentGroupReference> GetFoundGroups() => m_foundGroups;

        public override IDOMNode transformGroup(ICustomTransform.IImplementation genericImplementation,
            IDOMGroup group,
            ref bool changed, bool transformChildren, CTransformState state)
        {
            // Does this group have optional content information?
            var details = group.getOptionalContentDetails();
            if (details == null)
                return genericImplementation.transformGroup(null, group, ref changed, transformChildren, state);

            // What groups does this reference?
            using var referencedGroups = details.getGroupReferences().toVector();

            // Unfortunately for now we need to laboriously check to see if we've seen this before
            foreach (var referencedGroup in from referencedGroup in referencedGroups let found = m_foundGroups.Any(foundGroup => foundGroup.Equals(referencedGroup)) where !found select referencedGroup)
                m_foundGroups.Add(referencedGroup);

            // Descend further
            return genericImplementation.transformGroup(null, group, ref changed, transformChildren, state);
        }

        private readonly List<IOptionalContentGroupReference> m_foundGroups = [];
    }
}