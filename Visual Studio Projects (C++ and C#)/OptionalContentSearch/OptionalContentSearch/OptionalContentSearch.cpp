
/* -----------------------------------------------------------------------
 * <copyright file="OptionalContentSearch.cpp" company="Hybrid Software Helix Ltd">
 *  Copyright (c) 2025 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Hybrid Software Helix Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 * </summary>
 * -----------------------------------------------------------------------
 */

#include <iostream>
#include <jawsmako/jawsmako.h>
#include <jawsmako/pdfinput.h>
#include <jawsmako/customtransform.h>

using namespace JawsMako;

// In Mako, optional content is marked up either by IDOMGroup objects or annotations.
// Here we will check groups.
//
// This is complicated by the fact that optional content can be almost anywhere, including
// in patterns, soft masks, or glyphs in Type 3 fonts. So here to save some work we'll use
// a custom transform that will walk into every nook and cranny in the content for us, and
// avoid repeatedly revisiting shared resources.
class COptionalContentSearchImplementation : public ICustomTransform::IImplementation
{
public:
    void reset()
    {
        m_foundGroups.clear();
    }

    const COptionalContentGroupReferenceVect& getFoundGroups()
    {
        return m_foundGroups;
    }

    IDOMNodePtr transformGroup(IImplementation* genericImplementation, const IDOMGroupPtr& group, bool& changed, bool transformChildren, const CTransformState& state)
    {
        // Does this group have optional content information?
        const IOptionalContentDetailsPtr details = group->getOptionalContentDetails();
        if (details)
        {
            // What groups does this reference?
            COptionalContentGroupReferenceVect referencedGroups = details->getGroupReferences();

            // Unfortunately for now we need to laboriously check to see if we've seen this before
            for (const IOptionalContentGroupReferencePtr& referencedGroup : referencedGroups)
            {
                bool found = false;
                for (const IOptionalContentGroupReferencePtr& foundGroup : m_foundGroups)
                {
                    if (foundGroup->equals(referencedGroup))
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    m_foundGroups.append(referencedGroup);
                }
            }
        }

        // Descend further
        return genericImplementation->transformGroup(nullptr, group, changed, transformChildren, state);
    }


private:
    COptionalContentGroupReferenceVect m_foundGroups;
};



/**
 * Entry point for the command line tool.
 */
int main(const int argc, const char* argv[])
{
    U8String testFilePath = R"(..\..\TestFiles\)";

    try
    {
        const IJawsMakoPtr mako = IJawsMako::create();
        mako->enableAllFeatures(mako);
        const auto document = IPDFInput::create(mako)->open(testFilePath + argv[1])->getDocument();
        const auto optionalContent = document->getOptionalContent();

        // Create a custom transform to do our searching
        COptionalContentSearchImplementation optionalContentSearchImplementation;
        const ICustomTransformPtr optionalContentSearchTransform = ICustomTransform::create(mako, &optionalContentSearchImplementation);


        for (uint32 pageIndex = 0; pageIndex < document->getNumPages(); pageIndex++)
        {
            std::cout << "Page " << pageIndex + 1 << ":" << std::endl;

            // What groups does page 1 reference?
            {
                // Ask the transform. Note that here we want to clear caches between checks as we always
                // want the transform to descend into shared resources for each page. Caching would ordinarily
                // preclude that.
                //
                // We operate on a clone to ensure no changes to the tree. Custom transforms automatically
                // clean up duplicated resources which could cause an edit.
                optionalContentSearchImplementation.reset();
                optionalContentSearchTransform->flushCaches();
                optionalContentSearchTransform->transformPage(document->getPage()->clone());

                // So what groups do we have?
                COptionalContentGroupReferenceVect foundGroups = optionalContentSearchImplementation.getFoundGroups();
                for (const IOptionalContentGroupReferencePtr& foundGroup : foundGroups)
                {
                    IOptionalContentGroupPtr group = optionalContent->getGroup(foundGroup);

                    // This group is present - do something with this information
                    std::cout << "Found group: " << group->getName() << std::endl;
                }
            }
        }
    }
    catch (IError& e)
    {
        const String errorFormatString = getEDLErrorString(e.getErrorCode());
        std::wcerr << L"Mako exception thrown: " << e.getErrorDescription(errorFormatString) << std::endl;
        return 1;
    }
    catch (std::exception& e)
    {
        std::wcerr << L"std::exception thrown: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
