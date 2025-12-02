
/* -----------------------------------------------------------------------
 * <copyright file="Main.cpp" company="Hybrid Software Helix Ltd">
 *  Copyright (c) 2025 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  This example is provided on an "as is" basis and without warranty of any kind.
 *  Hybrid Software Helix Ltd. does not warrant or make any representations
 *  regarding the use or results of use of this example.
 * </summary>
 * -----------------------------------------------------------------------
 */

#include <iomanip>
#include <iostream>
#include <map>

#include <jawsmako/jawsmako.h>
#include <edl/idommetadata.h>

#include "xml/pugixml.hpp"

using namespace JawsMako;
using namespace EDL;

// Forward declarations
namespace
{
    uint32 readAllAvailable(const IInputStreamPtr& stream, uint8* ptr, uint32 size);
    bool loadStreamIntoEDLSysString(const IInputStreamPtr& stream, EDLSysString& string);
    std::map<EDLSysString, IDOMMetadata::eType> getMetadataFieldNames(const IDOMMetadataPtr& jobMetadata, enum IDOMMetadata::eType eType);
    std::map<EDLSysString, IDOMMetadata::eType> getAllMetadataFieldNames(const IDOMMetadataPtr& jobMetadata);
    String getMetadataValue(const IDOMMetadataPtr& jobMetadata, const EDLSysString& field, bool checkForValidFieldName = false);
    std::map<EDLSysString, String> getMetadataValues(const IDOMMetadataPtr& jobMetadata, enum IDOMMetadata::eType eType);
}

int main()
{
    U8String testFilePath = R"(..\..\TestFiles\)";

    try
    {
        const auto mako = IJawsMako::create();
        mako->enableAllFeatures(mako);
        const auto assembly = IInput::create(mako, eFFPDF)->open(testFilePath + "metadata-10ner.pdf");
        auto metadata = assembly->getJobMetadata();
        if (metadata == nullptr)
        {
            metadata = IDOMMetadata::create(mako);
            assembly->setJobMetadata(metadata);
        }

        // The following code first enumerates the fields available in the PDF for a given
        // category, then proceeds to display their values

        // Document info
        auto documentMetadata = getMetadataValues(metadata, IDOMMetadata::eDocumentInfo);
        if (documentMetadata.size() > 0)
        {
            std::wcout << L"\nDocument metadata:" << '\n';
            for (auto& [field, value] : documentMetadata)
                std::wcout << std::setw(25) << field.c_str() << L": " << value << std::endl;
        }

        // PDF info (these fields are readonly)
        auto pdfMetadata = getMetadataValues(metadata, IDOMMetadata::ePDFInfo);
        if (pdfMetadata.size() > 0)
        {
            std::wcout << L"\nPDF info metadata:" << '\n';
            for (auto& [field, value] : pdfMetadata)
                std::wcout << std::setw(25) << field.c_str() << L": " << value << std::endl;
        }

        // Viewer preferences
        auto prefsMetadata = getMetadataValues(metadata, IDOMMetadata::eViewerPreferences);
        if (prefsMetadata.size() > 0)
        {
            std::wcout << L"\nViewer preferences metadata:" << '\n';
            for (auto& [field, value] : prefsMetadata)
                std::wcout << std::setw(25) << field.c_str() << L": " << value << std::endl;
        }

        // Page view preferences
        auto pageViewMetadata = getMetadataValues(metadata, IDOMMetadata::ePageView);
        if (pageViewMetadata.size() > 0)
        {
            std::wcout << L"\nPage view preferences metadata:" << '\n';
            for (auto& [field, value] : pageViewMetadata)
                std::wcout << std::setw(25) << field.c_str() << L": " << value << std::endl;
        }

        // XMP
        EDLSysString xmpPacket;
        loadStreamIntoEDLSysString(edlobj2IRAInputStream(assembly->getXmpPacket()), xmpPacket);

        // Parse XML
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_string(xmpPacket.c_str());
        if (result)
        {
            pugi::xml_node rdf = doc.child("x:xmpmeta").child("rdf:RDF").child("rdf:Description");
            std::cout << "\nXMP Packet" << '\n';
            for (auto node = rdf.first_child(); node; node = node.next_sibling())
            {
                std::string name = node.name();
                std::cout << std::setw(25) << name << ": ";
                if (name.find("dc:") != std::string::npos)
                {
                    auto contentNode = node.child("rdf:Alt").child("rdf:li");
                    if (contentNode)
                    {
                        std::cout << contentNode.text().as_string() << '\n';
                        continue;
                    }

                    contentNode = node.child("rdf:Bag").child("rdf:li");
                    if (contentNode)
                    {
                        std::cout << contentNode.text().as_string() << '\n';
                        continue;
                    }

                    std::cout << node.text().as_string() << '\n';
                }
                // change the value of a property - for example "CaptionWriter"
                else if (name.find("photoshop:CaptionWriter") != std::string::npos)
                {
                    node.text().set("New value");
                    std::cout << node.text().as_string() << '\n';
                }
                else
                    std::cout << node.text().as_string() << '\n';
            }
        }
    }
    catch (IError& e)
    {
        const String errorFormatString = getEDLErrorString(e.getErrorCode());
        std::wcerr << L"Exception thrown: " << e.getErrorDescription(errorFormatString) << std::endl;
        return static_cast<int>(e.getErrorCode());
    }
    catch (std::exception& e)
    {
        std::wcerr << L"std::exception thrown: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

namespace
{
    // Fill a buffer as much as possible
    uint32 readAllAvailable(const IInputStreamPtr& stream, uint8* ptr, uint32 size)
    {
        uint32 amountRead = 0;
        while (size)
        {
            int32 result = stream->read(ptr, static_cast<int>(size));
            if (result < 0)
                return 0;

            if (result == 0)
                return amountRead;

            amountRead += result;
            ptr += result;
            size -= result;
        }
        return amountRead;
    }

    // Function to load everything into a string
    bool loadStreamIntoEDLSysString(const IInputStreamPtr& stream, EDLSysString& string)
    {
        if (!stream)
            return false;

        // Open the stream
        if (!stream->open())
            return false;

        // For the sake of efficiency, there are two paths here
        // If this is an RA input string, then we can find the size
        IRAInputStreamPtr raStream = edlobj2IRAInputStream(stream);
        if (raStream)
        {
            int64_t size = raStream->length();

            // Check for sensible range
            if (size < 0 || size > 0x7fffffff)
                return false;


            // Size the output
            string.resize(static_cast<uint32_t>(size));

            // And read!
            if (size && readAllAvailable(stream, reinterpret_cast<uint8_t*>(string.data()), static_cast<uint32_t>(size)) != size)
                // Didn't get all we were expecting.
                return false;

        }
        else
        {
            // Well, do things an inefficient way with constant reallocs
            // We'll allocate 10K each time round the loop to keep it reasonable.
            uint32_t result;
            string.resize(0);

            do
            {
                // Try for another 100K
                size_t oldSize = string.size();
                string.resize(oldSize + 102400);

                // Again, should be valid according to the spec
                const auto ptr = reinterpret_cast<uint8_t*>(&string[oldSize]);

                // Read as much as we can
                result = readAllAvailable(stream, ptr, 102400);

                // Resize to the result to keep accounting simple
                string.resize(oldSize + result);
            } while (result);
        }

        // Close the stream
        stream->close();

        // Done
        return true;
    }


    // Get metadata field names
    std::map<EDLSysString, IDOMMetadata::eType> getMetadataFieldNames(const IDOMMetadataPtr& jobMetadata, enum IDOMMetadata::eType eType)
    {
        std::map<EDLSysString, IDOMMetadata::eType> fieldNames;
        auto metadataFieldNames = jobMetadata->getPropertyCollectionEnum(eType);
        if (metadataFieldNames)
        {
            EDLSysString fieldName;
            for (uint32_t i = 0; i < metadataFieldNames->count(); i++)
            {
                metadataFieldNames->getNext(&fieldName);
                fieldNames.insert(std::make_pair(fieldName, eType));
            }
        }

        return fieldNames;
    }

    // Get all metadata field names
    std::map<EDLSysString, IDOMMetadata::eType> getAllMetadataFieldNames(const IDOMMetadataPtr& jobMetadata)
    {
        std::map<EDLSysString, IDOMMetadata::eType> allFieldNames;
        for (const auto eType : { IDOMMetadata::eDocumentInfo, IDOMMetadata::ePDFInfo, IDOMMetadata::eViewerPreferences, IDOMMetadata::ePageView })
            for (const auto& fieldName : getMetadataFieldNames(jobMetadata, eType))
                allFieldNames.insert(fieldName);

        return allFieldNames;
    }

    // Return a named metadata string
    String getMetadataValue(const IDOMMetadataPtr& jobMetadata, const EDLSysString& field, bool checkForValidFieldName)
    {
        PValue pVal;
        const auto allFieldNames = getAllMetadataFieldNames(jobMetadata);
        if (checkForValidFieldName)
        {
            if (allFieldNames.find(field) == allFieldNames.end())
                return L"Field name not recognized";
        }

        jobMetadata->getProperty(allFieldNames.find(field)->second, field, pVal);

        if (pVal.getType() == PValue::T_UNASSIGNED)
            return L"** Not set (or unknown) **";

        if (pVal.getType() == PValue::T_BOOL)
            return pVal.getBool() ? L"True" : L"False";

        if (pVal.getType() == PValue::T_STRINGVECT)
        {
            CStringVect valueList = pVal.getStringVect();
            String allValues;
            for (const String& valueItem : valueList)
            {
                allValues += valueItem;
                if (&valueItem != &valueList.last())
                    allValues += L"; ";
            }
            return allValues;
        }

        if (pVal.getType() == PValue::T_STRING)
            return pVal.getString();

        if (pVal.getType() == PValue::T_TIME)
            return U8StringToString(pVal.getTime()->toW3CDTF());

        return L"";
    }

    // Return a map of job metadata
    std::map<EDLSysString, String> getMetadataValues(const IDOMMetadataPtr& jobMetadata, enum IDOMMetadata::eType eType)
    {
        // Build map
        const auto fieldNames = getMetadataFieldNames(jobMetadata, eType);
        std::map<EDLSysString, String> returnList;
        for (const auto& fieldName : fieldNames)
            returnList.insert(std::make_pair(fieldName.first, getMetadataValue(jobMetadata, fieldName.first)));

        return returnList;
    }
}