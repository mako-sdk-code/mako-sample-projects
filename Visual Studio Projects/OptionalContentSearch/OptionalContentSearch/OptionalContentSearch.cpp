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

#include <filesystem>
#include <iostream>
#include <jawsmako/jawsmako.h>
#include <jawsmako/pdfinput.h>
#include <jawsmako/customtransform.h>

using namespace JawsMako;

const U8String TEST_FILES_PATH = R"(..\..\..\..\TestFiles\)";

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

    IDOMNodePtr transformGroup(IImplementation* genericImplementation, const IDOMGroupPtr& group, bool& changed, bool transformChildren, const CTransformState& state) override
    {
        // Does this group have optional content information?
        if (const IOptionalContentDetailsPtr details = group->getOptionalContentDetails())
        {
            // Unfortunately for now we need to laboriously check to see if we've seen this before
            for (auto referencedGroups = details->getGroupReferences(); const IOptionalContentGroupReferencePtr& referencedGroup : referencedGroups)
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
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <input.pdf>" << '\n';
        return 1;
    }

    try
    {
        const IJawsMakoPtr mako = IJawsMako::create();
        IJawsMako::enableAllFeatures(mako);
        const auto document = IPDFInput::create(mako)->open(TEST_FILES_PATH + argv[1])->getDocument();
        const auto optionalContent = document->getOptionalContent();

        // Create a custom transform to do our searching
        COptionalContentSearchImplementation optionalContentSearchImplementation;
        const auto optionalContentSearchTransform = ICustomTransform::create(mako, &optionalContentSearchImplementation);


        for (uint32 pageIndex = 0; pageIndex < document->getNumPages(); pageIndex++)
        {
            std::cout << "Page " << pageIndex + 1 << ":" << '\n';

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
                optionalContentSearchTransform->transformPage(document->getPage(pageIndex)->clone());

                // So what groups do we have?
                for (auto foundGroups = optionalContentSearchImplementation.getFoundGroups(); const IOptionalContentGroupReferencePtr& foundGroup : foundGroups)
                {
                    auto group = optionalContent->getGroup(foundGroup);

                    // This group is present - do something with this information
                    std::cout << "Found group: " << group->getName() << '\n';
                }
            }
        }
    }
    catch (IError& e)
    {
        const String errorFormatString = getEDLErrorString(e.getErrorCode());
        std::wcerr << L"Mako exception thrown: " << e.getErrorDescription(errorFormatString) << '\n';
        return 1;
    }
    catch (std::exception& e)
    {
        std::wcerr << L"std::exception thrown: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
