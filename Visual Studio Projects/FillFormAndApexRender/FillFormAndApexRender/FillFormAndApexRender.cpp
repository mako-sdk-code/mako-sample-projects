/* -----------------------------------------------------------------------
 * <copyright file="FillFormAndApexRender.cpp" company="Hybrid Software Helix Ltd">
 *  Copyright (C) 2026 Hybrid Software Helix Ltd. All rights reserved.
 * </copyright>
 * <summary>
 *  Demonstrates filling PDF form fields, creating widget appearances, and rendering
 *  the resulting page with Apex.
 * </summary>
 * -----------------------------------------------------------------------
 */

#include <iostream>
#include <algorithm>
#include <cctype>
#include <set>
#include <string>
#include <vector>

#include <jawsmako/apexrenderer.h>
#include <jawsmako/interactive.h>
#include <jawsmako/jawsmako.h>
#include <edl/idomglyphs.h>

using namespace JawsMako;
using namespace EDL;

namespace
{
    constexpr double POINTS_PER_INCH = 72.0;
    constexpr double XPS_UNITS_PER_INCH = 96.0;

    double ptToXps(const double value)
    {
        return value / POINTS_PER_INCH * XPS_UNITS_PER_INCH;
    }

    std::pair<IDOMFontOpenTypePtr, uint32> getArial(const IJawsMakoPtr& mako)
    {
        uint32 fontIndex = 0;
        return { edlobj2IDOMFontOpenType(mako->findFont("Arial", fontIndex)), fontIndex };
    }

    void addLabel(const IJawsMakoPtr& mako,
                  const IDOMFixedPagePtr& content,
                  const IDOMFontOpenTypePtr& font,
                  const uint32 fontIndex,
                  const EDLString& text,
                  const FPoint& origin)
    {
        const auto black = IDOMSolidColorBrush::createSolidRgb(mako, 0.0f, 0.0f, 0.0f);
        content->appendChild(IDOMGlyphs::create(mako, text, ptToXps(10.0), origin, black, font, fontIndex));
    }

    void createFormTemplate(const IJawsMakoPtr& mako, const U8String& outputPdf)
    {
        const auto assembly = IDocumentAssembly::create(mako);
        const auto document = IDocument::create(mako);
        assembly->appendDocument(document);

        const auto page = IPage::create(mako);
        document->appendPage(page);

        const auto content = IDOMFixedPage::create(mako, 8.5 * XPS_UNITS_PER_INCH, 11.0 * XPS_UNITS_PER_INCH);
        page->setContent(content);

        const auto form = IForm::create(mako);
        document->setForm(form);

        const auto [font, fontIndex] = getArial(mako);

        addLabel(mako, content, font, fontIndex, L"Customer name", FPoint(72.0, 92.0));
        addLabel(mako, content, font, fontIndex, L"Job number", FPoint(72.0, 140.0));
        addLabel(mako, content, font, fontIndex, L"Approved", FPoint(72.0, 188.0));

        const auto customerName = IWidgetAnnotation::createTextField(
            mako, form, page, FRect(190.0, 74.0, 250.0, 28.0), "CustomerName", "", 10.0f, font, fontIndex);
        customerName->createBasicAppearances(form, "", font, fontIndex, 10.0f);

        const auto jobNumber = IWidgetAnnotation::createTextField(
            mako, form, page, FRect(190.0, 122.0, 160.0, 28.0), "JobNumber", "", 10.0f, font, fontIndex);
        jobNumber->createBasicAppearances(form, "", font, fontIndex, 10.0f);

        const auto approved = IWidgetAnnotation::createCheckButton(
            mako, form, page, FRect(190.0, 172.0, 18.0, 18.0), "Approved", "Yes", false);
        approved->createBasicAppearances(form, "Yes", font, fontIndex, 10.0f);

        IOutput::create(mako, eFFPDF)->writeAssembly(assembly, outputPdf);
    }

    struct PromptField
    {
        std::string displayName;
        IFormFieldPtr field;
        IWidgetAnnotationPtr widget;
    };

    std::string toString(const U8String& value)
    {
        return value.c_str();
    }

    std::string preferredName(const IFormFieldPtr& field)
    {
        auto name = toString(field->getPartialName());
        if (name.empty())
            name = toString(field->getAlternateName());
        return name;
    }

    std::string preferredName(const IWidgetAnnotationPtr& widget)
    {
        auto name = toString(widget->getPartialName());
        if (name.empty())
            name = toString(widget->getAlternateName());
        return name;
    }

    std::string qualifiedName(const std::string& parentName, const std::string& name)
    {
        if (parentName.empty())
            return name;
        if (name.empty())
            return parentName;
        return parentName + "." + name;
    }

    std::string valueToString(const CU8StringVect& values)
    {
        std::string result;
        for (const auto& value : values)
        {
            if (!result.empty())
                result += ", ";
            result += value.c_str();
        }
        return result;
    }

    bool equalsIgnoreCase(const std::string& lhs, const std::string& rhs)
    {
        return lhs.size() == rhs.size()
            && std::equal(lhs.begin(), lhs.end(), rhs.begin(),
                [](const char a, const char b)
                {
                    return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
                });
    }

    std::string getOnStateName(const IWidgetAnnotationPtr& widget)
    {
        if (!widget)
            return {};

        for (const auto& appearance : widget->getAppearances())
            if (const auto state = toString(appearance->getState()); !state.empty() && !equalsIgnoreCase(state, "Off"))
                return state;

        return {};
    }

    std::string currentValue(const PromptField& promptField)
    {
        CU8StringVect values;
        bool hasValue = false;
        if (promptField.field)
            hasValue = promptField.field->getValue(values);
        else if (promptField.widget)
            hasValue = promptField.widget->getValue(values);

        return hasValue ? valueToString(values) : std::string();
    }

    U8String widgetAppearanceState(const IWidgetAnnotationPtr& widget)
    {
        CU8StringVect values;
        if (widget->getValue(values) && !values.empty())
            return values[0];

        return widget->getState();
    }

    void addFieldPrompts(const IFormFieldPtr& field,
                         const std::string& parentName,
                         std::vector<PromptField>& promptFields,
                         std::set<std::string>& seenNames)
    {
        const auto name = preferredName(field);
        const auto displayName = qualifiedName(parentName, name);
        const bool hasChildFields = !field->getChildFields().empty();

        if (const bool hasChildWidgets = !field->getChildWidgets().empty(); !displayName.empty() && (hasChildWidgets || !hasChildFields) && seenNames.insert(displayName).second)
            promptFields.push_back({ displayName, field, IWidgetAnnotationPtr() });

        for (const auto& child : field->getChildFields())
            addFieldPrompts(child, displayName, promptFields, seenNames);
    }

    std::vector<PromptField> discoverPromptFields(const IDocumentPtr& document)
    {
        const auto form = document->getForm();
        if (!form)
            throw std::runtime_error("The input PDF does not contain an interactive form.");

        std::vector<PromptField> promptFields;
        std::set<std::string> seenNames;

        for (const auto& field : form->getFields())
            addFieldPrompts(field, std::string(), promptFields, seenNames);

        for (const auto& widgetRef : form->getWidgets())
        {
            const auto annotation = document->findAnnotation(widgetRef);
            if (!annotation || annotation->getType() != IAnnotation::eATWidget)
                continue;

            const auto widget = obj2IWidgetAnnotation(annotation);
            if (const auto displayName = preferredName(widget); !displayName.empty() && seenNames.insert(displayName).second)
                promptFields.push_back({ displayName, IFormFieldPtr(), widget });
        }

        if (promptFields.empty())
            throw std::runtime_error("No fillable form fields were found.");

        return promptFields;
    }

    void promptForFieldValues(const IDocumentPtr& document)
    {
        const auto promptFields = discoverPromptFields(document);

        std::cout << "Detected " << promptFields.size() << " fillable form field";
        std::cout << (promptFields.size() == 1 ? "." : "s.") << '\n';
        std::cout << "Press Enter without typing a value to leave a field unchanged.\n\n";

        for (const auto& promptField : promptFields)
        {
            const auto existingValue = currentValue(promptField);
            std::cout << promptField.displayName;
            if (!existingValue.empty())
                std::cout << " [" << existingValue << "]";
            if (promptField.widget)
            {
                if (auto onState = getOnStateName(promptField.widget); !onState.empty())
                    std::cout << " (on value: " << onState << ")";
            }
            std::cout << ": ";

            std::string input;
            std::getline(std::cin, input);
            if (input.empty())
                continue;

            if (promptField.field)
                promptField.field->setValue(input.c_str());
            else if (promptField.widget)
            {
                promptField.widget->setValue(input.c_str());
                if (equalsIgnoreCase(input, getOnStateName(promptField.widget)))
                    promptField.widget->setState(input.c_str());
            }
        }
    }

    void updateWidgetAppearances(const IJawsMakoPtr& mako, const IDocumentPtr& document)
    {
        const auto form = document->getForm();
        if (!form)
            return;

        const auto [font, fontIndex] = getArial(mako);
        for (uint32 pageIndex = 0; pageIndex < document->getNumPages(); ++pageIndex)
        {
            const auto page = document->getPage(pageIndex);
            for (const auto& annotation : page->getAnnotations())
            {
                if (annotation->getType() != IAnnotation::eATWidget)
                    continue;

                const auto widget = obj2IWidgetAnnotation(annotation);
                if (!widget)
                    continue;

                try
                {
                    widget->createBasicAppearances(form, getOnStateName(widget).c_str(), font, fontIndex, 10.0f);
                }
                catch (IError&)
                {
                    std::wcerr << L"Warning: basic appearance generation is not supported for one widget.\n";
                }
            }
        }
    }

    void appendWidgetAppearancesToPageContent(const IPagePtr& page)
    {
        const auto content = page->getContent();
        for (const auto& annotation : page->getAnnotations())
        {
            if (annotation->getType() != IAnnotation::eATWidget)
                continue;

            const auto widget = obj2IWidgetAnnotation(annotation);
            try
            {
                const auto appearance = annotation->getAppearance(eAUNormal, widgetAppearanceState(widget));
                content->appendChild(appearance->getScaledAppearance(annotation->getRect()));
            }
            catch (IError&)
            {
                for (const auto& appearance : annotation->getAppearances())
                {
                    content->appendChild(appearance->getScaledAppearance(annotation->getRect()));
                    break;
                }
            }
        }
    }

    void renderPageWithApex(const IJawsMakoPtr& mako, const IPagePtr& page, const U8String& outputPng)
    {
        const auto cropBox = page->getCropBox();
        const auto content = page->getContent();

        CImageRenderSpec renderSpec;
        renderSpec.width = static_cast<uint32>(cropBox.dX / XPS_UNITS_PER_INCH * 300.0);
        renderSpec.height = static_cast<uint32>(cropBox.dY / XPS_UNITS_PER_INCH * 300.0);
        renderSpec.sourceRect = cropBox;
        renderSpec.processSpace = IDOMColorSpaceDeviceRGB::create(mako);

        const auto renderer = IApexRenderer::create(mako);
        renderer->render(content, &renderSpec);

        IDOMPNGImage::encode(mako, renderSpec.result, IOutputStream::createToFile(mako, outputPng));
    }

    void usage()
    {
        std::wcout
            << L"Usage:\n"
            << L"  FillFormAndApexRender.exe [input.pdf] [filled-output.pdf] [render-output.png]\n\n"
            << L"For checkboxes/radio buttons, enter the field's export value, such as Yes.\n"
            << L"If input.pdf is omitted, the sample first creates FormTemplate.pdf with fields named\n"
            << L"CustomerName, JobNumber, and Approved, then prompts for values and renders that template.\n";
    }
}

int main(int argc, char** argv)
{
    try
    {
        const auto mako = IJawsMako::create();
        IJawsMako::enableAllFeatures(mako);

        U8String inputPdf = argc > 1 ? argv[1] : "FormTemplate.pdf";
        const U8String filledPdf = argc > 2 ? argv[2] : "FilledForm.pdf";
        const U8String renderedPng = argc > 3 ? argv[3] : "FilledForm.png";

        if (argc == 1)
        {
            usage();
            std::wcout << L"\nCreating FormTemplate.pdf.\n";
            createFormTemplate(mako, inputPdf);
        }

        const auto assembly = IInput::create(mako, eFFPDF)->open(inputPdf);
        const auto document = assembly->getDocument();

        promptForFieldValues(document);
        updateWidgetAppearances(mako, document);
        IOutput::create(mako, eFFPDF)->writeAssembly(assembly, filledPdf);

        if (argc == 1 || argc > 3)
        {
            const auto firstPage = document->getPage(0);
            appendWidgetAppearancesToPageContent(firstPage);
            renderPageWithApex(mako, firstPage, renderedPng);
        }

        std::cout << "Wrote " << filledPdf.c_str();
        if (argc == 1 || argc > 3)
            std::cout << " and " << renderedPng.c_str();
        std::cout << ".\n";
        return 0;
    }
    catch (IError& e)
    {
        const String errorFormatString = getEDLErrorString(e.getErrorCode());
        std::wcerr << L"Exception thrown: " << e.getErrorDescription(errorFormatString) << L'\n';
        return 1;
    }
    catch (std::exception& e)
    {
        std::wcerr << L"std::exception thrown: " << e.what() << L'\n';
        return 1;
    }
}
