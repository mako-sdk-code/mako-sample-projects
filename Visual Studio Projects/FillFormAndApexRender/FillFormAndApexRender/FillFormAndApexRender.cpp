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
#include <sstream>
#include <stdexcept>
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

    struct DefaultAppearanceStyle
    {
        IDOMFontPtr font;
        uint32 fontIndex = 0;
        float fontSize = 10.0f;
        bool usedFallback = false;
        std::string warning;
    };

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
        addLabel(mako, content, font, fontIndex, L"Imported image", FPoint(72.0, 238.0));

        const auto customerName = IWidgetAnnotation::createTextField(
            mako, form, page, FRect(190.0, 74.0, 250.0, 28.0), "CustomerName", "", 10.0f, font, fontIndex);
        customerName->createBasicAppearances(form, "", font, fontIndex, 10.0f);

        const auto jobNumber = IWidgetAnnotation::createTextField(
            mako, form, page, FRect(190.0, 122.0, 160.0, 28.0), "JobNumber", "", 10.0f, font, fontIndex);
        jobNumber->createBasicAppearances(form, "", font, fontIndex, 10.0f);

        const auto approved = IWidgetAnnotation::createCheckButton(
            mako, form, page, FRect(190.0, 172.0, 18.0, 18.0), "Approved", "Yes", false);
        approved->createBasicAppearances(form, "Yes", font, fontIndex, 10.0f);

        const auto imageActions = IPDFDictionary::create(mako, 2);
        imageActions->putName("S", "JavaScript");
        imageActions->putString("JS", "event.target.buttonImportIcon();");

        const auto imageButton = IWidgetAnnotation::createButton(
            mako,
            form,
            page,
            FRect(190.0, 220.0, 160.0, 90.0),
            "ImportedImage",
            "Choose image",
            10.0f,
            font,
            fontIndex,
            IWidgetAnnotation::eFELatin,
            IDOMColor::createSolidRgb(mako, 0.0f, 0.0f, 0.0f),
            IDOMColor::createSolidRgb(mako, 0.94f, 0.94f, 0.94f),
            imageActions,
            "Imported image");
        imageButton->createBasicAppearances(form, "", font, fontIndex, 10.0f);

        IOutput::create(mako, eFFPDF)->writeAssembly(assembly, outputPdf);
    }

    enum class PromptFieldKind
    {
        Value,
        ImageButton
    };

    struct PromptField
    {
        std::string displayName;
        IFormFieldPtr field;
        IWidgetAnnotationPtr widget;
        PromptFieldKind kind = PromptFieldKind::Value;
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

    bool endsWithIgnoreCase(const std::string& value, const std::string& suffix)
    {
        return value.size() >= suffix.size()
            && std::equal(suffix.rbegin(), suffix.rend(), value.rbegin(),
                [](const char a, const char b)
                {
                    return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
                });
    }

    std::string trimMatchingQuotes(const std::string& value)
    {
        if (value.size() >= 2 && ((value.front() == '"' && value.back() == '"') || (value.front() == '\'' && value.back() == '\'')))
            return value.substr(1, value.size() - 2);

        return value;
    }

    bool tryParseFloat(const std::string& value, float& result)
    {
        try
        {
            size_t parsedChars = 0;
            const auto parsedValue = std::stof(value, &parsedChars);
            if (parsedChars == value.size() && std::isfinite(parsedValue) && parsedValue > 0.0f)
            {
                result = parsedValue;
                return true;
            }
        }
        catch (std::exception&)
        {
        }

        return false;
    }

    bool parseDefaultAppearanceStyle(const std::string& defaultAppearanceString,
                                     std::string& fontResourceName,
                                     float& fontSize)
    {
        std::istringstream stream(defaultAppearanceString);
        std::vector<std::string> tokens;
        std::string token;
        while (stream >> token)
            tokens.push_back(token);

        for (auto index = tokens.size(); index-- > 2;)
        {
            if (tokens[index] != "Tf")
                continue;

            auto candidateFontName = tokens[index - 2];
            if (!candidateFontName.empty() && candidateFontName.front() == '/')
                candidateFontName.erase(candidateFontName.begin());

            float candidateFontSize = 0.0f;
            if (!candidateFontName.empty() && tryParseFloat(tokens[index - 1], candidateFontSize))
            {
                fontResourceName = candidateFontName;
                fontSize = candidateFontSize;
                return true;
            }
        }

        return false;
    }

    bool tryFindFont(const IJawsMakoPtr& mako,
                     const std::string& fontName,
                     IDOMFontPtr& font,
                     uint32& fontIndex)
    {
        if (fontName.empty() || !mako->findFont(fontName.c_str()))
            return false;

        fontIndex = 0;
        font = mako->findFont(fontName.c_str(), fontIndex);
        return !!font;
    }

    bool tryFindFontFromCandidates(const IJawsMakoPtr& mako,
                                   const std::vector<std::string>& fontNames,
                                   IDOMFontPtr& font,
                                   uint32& fontIndex)
    {
        for (const auto& fontName : fontNames)
        {
            if (tryFindFont(mako, fontName, font, fontIndex))
                return true;
        }

        return false;
    }

    bool resolveAppearanceFont(const IJawsMakoPtr& mako,
                               const std::string& fontResourceName,
                               IDOMFontPtr& font,
                               uint32& fontIndex)
    {
        if (tryFindFont(mako, fontResourceName, font, fontIndex))
            return true;

        if (equalsIgnoreCase(fontResourceName, "Helv"))
            return tryFindFontFromCandidates(mako, { "Helvetica", "Arial" }, font, fontIndex);
        if (equalsIgnoreCase(fontResourceName, "TiRo"))
            return tryFindFontFromCandidates(mako, { "Times-Roman", "Times New Roman" }, font, fontIndex);
        if (equalsIgnoreCase(fontResourceName, "Cour"))
            return tryFindFontFromCandidates(mako, { "Courier", "Courier New" }, font, fontIndex);

        return false;
    }

    DefaultAppearanceStyle getDefaultAppearanceStyle(const IJawsMakoPtr& mako,
                                                     const IFormPtr& form,
                                                     const IWidgetAnnotationPtr& widget)
    {
        DefaultAppearanceStyle style;
        const auto [fallbackFont, fallbackFontIndex] = getArial(mako);
        style.font = fallbackFont;
        style.fontIndex = fallbackFontIndex;

        std::string defaultAppearanceString;
        try
        {
            defaultAppearanceString = toString(form->getWidgetDefaultAppearanceString(widget));
        }
        catch (IError&)
        {
        }

        std::string fontResourceName;
        float parsedFontSize = 0.0f;
        if (!parseDefaultAppearanceStyle(defaultAppearanceString, fontResourceName, parsedFontSize))
        {
            style.usedFallback = true;
            style.warning = "could not parse default appearance; using Arial 10.";
            return style;
        }

        style.fontSize = parsedFontSize;
        if (!resolveAppearanceFont(mako, fontResourceName, style.font, style.fontIndex))
        {
            style.usedFallback = true;
            style.font = fallbackFont;
            style.fontIndex = fallbackFontIndex;
            style.warning = "could not resolve default appearance font '" + fontResourceName + "'; using Arial with size "
                + std::to_string(static_cast<double>(style.fontSize)) + ".";
        }

        return style;
    }

    bool shouldWarnForDefaultAppearanceFallback(const IFormPtr& form, const IWidgetAnnotationPtr& widget)
    {
        try
        {
            const auto fieldType = form->getWidgetFieldType(widget);
            return fieldType == eFTText || fieldType == eFTChoice;
        }
        catch (IError&)
        {
            return widget->getFieldType() == eFTText || widget->getFieldType() == eFTChoice;
        }
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

    bool pdfStringContains(const IPDFObjectPtr& object, const std::string& needle)
    {
        if (!object || object->getType() != ePOTString)
            return false;

        const auto pdfString = obj2IPDFString(object);
        return pdfString && toString(pdfString->getTextValue()).find(needle) != std::string::npos;
    }

    bool pdfNameEquals(const IPDFObjectPtr& object, const char* expected)
    {
        if (!object || object->getType() != ePOTName)
            return false;

        const auto pdfName = obj2IPDFName(object);
        return pdfName && pdfName->getValue() == expected;
    }

    bool pdfObjectContainsButtonImportIconAction(const IPDFObjectPtr& object, const uint32 depth = 0)
    {
        if (!object || depth > 8)
            return false;

        if (object->getType() == ePOTDictionary)
        {
            const auto dictionary = obj2IPDFDictionary(object);
            if (!dictionary)
                return false;

            if (pdfNameEquals(dictionary->get("S"), "JavaScript") && pdfStringContains(dictionary->get("JS"), "buttonImportIcon"))
                return true;

            for (uint32 index = 0; index < dictionary->getSize(); ++index)
            {
                const auto value = dictionary->getValueAtIndex(index);
                if (pdfObjectContainsButtonImportIconAction(value, depth + 1))
                    return true;
            }
        }
        else if (object->getType() == ePOTArray)
        {
            const auto array = obj2IPDFArray(object);
            if (!array)
                return false;

            for (uint32 index = 0; index < array->getSize(); ++index)
            {
                if (pdfObjectContainsButtonImportIconAction(array->get(index), depth + 1))
                    return true;
            }
        }

        return false;
    }

    bool dictionaryContainsButtonImportIconAction(const IPDFDictionaryConstPtr& dictionary)
    {
        if (!dictionary)
            return false;

        if (pdfNameEquals(dictionary->get("S"), "JavaScript") && pdfStringContains(dictionary->get("JS"), "buttonImportIcon"))
            return true;

        for (uint32 index = 0; index < dictionary->getSize(); ++index)
        {
            if (pdfObjectContainsButtonImportIconAction(dictionary->getValueAtIndex(index)))
                return true;
        }

        return false;
    }

    bool isImageButtonWidget(const IFormPtr& form, const IWidgetAnnotationPtr& widget)
    {
        if (!widget)
            return false;

        try
        {
            if (form->getWidgetFieldType(widget) != eFTButton)
                return false;

            if ((form->getWidgetFieldFlags(widget) & eFFPushButton) == 0)
                return false;
        }
        catch (IError&)
        {
            uint32 flags = 0;
            if (widget->getFieldType() != eFTButton || !widget->getFieldFlags(flags) || (flags & eFFPushButton) == 0)
                return false;
        }

        return dictionaryContainsButtonImportIconAction(widget->getActionsDictionary())
            || dictionaryContainsButtonImportIconAction(widget->getAdditionalActionsDictionary());
    }

    std::string widgetQualifiedName(const IFormPtr& form, const IWidgetAnnotationPtr& widget)
    {
        std::string parentName;
        try
        {
            for (const auto& parent : form->getPathToWidget(widget))
                parentName = qualifiedName(parentName, preferredName(parent));
        }
        catch (IError&)
        {
        }

        return qualifiedName(parentName, preferredName(widget));
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
            promptFields.push_back({ displayName, field, IWidgetAnnotationPtr(), PromptFieldKind::Value });

        for (const auto& child : field->getChildFields())
            addFieldPrompts(child, displayName, promptFields, seenNames);
    }

    std::vector<PromptField> discoverPromptFields(const IDocumentPtr& document)
    {
        const auto form = document->getForm();
        if (!form)
            throw std::runtime_error("The input PDF does not contain an interactive form.");

        std::vector<PromptField> promptFields;
        std::vector<PromptField> imageButtonFields;
        std::set<std::string> seenNames;

        for (const auto& widgetRef : form->getWidgets())
        {
            const auto annotation = document->findAnnotation(widgetRef);
            if (!annotation || annotation->getType() != IAnnotation::eATWidget)
                continue;

            const auto widget = obj2IWidgetAnnotation(annotation);
            if (!isImageButtonWidget(form, widget))
                continue;

            const auto displayName = widgetQualifiedName(form, widget);
            if (!displayName.empty() && seenNames.insert(displayName).second)
                imageButtonFields.push_back({ displayName, IFormFieldPtr(), widget, PromptFieldKind::ImageButton });
        }

        for (const auto& field : form->getFields())
            addFieldPrompts(field, std::string(), promptFields, seenNames);

        for (const auto& widgetRef : form->getWidgets())
        {
            const auto annotation = document->findAnnotation(widgetRef);
            if (!annotation || annotation->getType() != IAnnotation::eATWidget)
                continue;

            const auto widget = obj2IWidgetAnnotation(annotation);
            if (const auto displayName = widgetQualifiedName(form, widget); !displayName.empty() && seenNames.insert(displayName).second)
                promptFields.push_back({ displayName, IFormFieldPtr(), widget, PromptFieldKind::Value });
        }

        promptFields.insert(promptFields.end(), imageButtonFields.begin(), imageButtonFields.end());

        if (promptFields.empty())
            throw std::runtime_error("No fillable form fields were found.");

        return promptFields;
    }

    IDOMImagePtr createImageFromStream(const IJawsMakoPtr& mako, const IInputStreamPtr& imageStream, const std::string& sourceName)
    {
        if (endsWithIgnoreCase(sourceName, ".png"))
            return IDOMPNGImage::create(mako, imageStream);
        if (endsWithIgnoreCase(sourceName, ".jpg") || endsWithIgnoreCase(sourceName, ".jpeg"))
            return IDOMJPEGImage::create(mako, imageStream);
        if (endsWithIgnoreCase(sourceName, ".tif") || endsWithIgnoreCase(sourceName, ".tiff"))
            return IDOMTIFFImage::create(mako, imageStream);

        throw std::runtime_error("Unsupported image type. Use PNG, JPEG, or TIFF.");
    }

    void setImageButtonAppearanceFromStream(const IJawsMakoPtr& mako,
                                            const IWidgetAnnotationPtr& widget,
                                            const IInputStreamPtr& imageStream,
                                            const std::string& sourceName)
    {
        const auto image = createImageFromStream(mako, imageStream, sourceName);
        const auto rect = widget->getRect();
        const auto form = IDOMForm::create(mako, FMatrix(), FRect(0.0, 0.0, rect.dX, rect.dY));
        form->appendChild(IDOMPathNode::createImage(mako, image, FRect(0.0, 0.0, rect.dX, rect.dY)));
        widget->addAppearance(IAnnotationAppearance::create(mako, form, eAUNormal));
    }

    void setImageButtonAppearanceFromPath(const IJawsMakoPtr& mako,
                                          const IWidgetAnnotationPtr& widget,
                                          const std::string& imagePath)
    {
        const auto normalizedPath = trimMatchingQuotes(imagePath);
        setImageButtonAppearanceFromStream(
            mako,
            widget,
            IInputStream::createFromFile(mako, normalizedPath.c_str()),
            normalizedPath);
    }

    void promptForFieldValues(const IJawsMakoPtr& mako, const IDocumentPtr& document)
    {
        const auto promptFields = discoverPromptFields(document);

        std::cout << "Detected " << promptFields.size() << " fillable form field";
        std::cout << (promptFields.size() == 1 ? "." : "s.") << '\n';
        std::cout << "Press Enter without typing a value to leave a field unchanged.\n\n";

        for (const auto& promptField : promptFields)
        {
            const auto existingValue = currentValue(promptField);
            std::cout << promptField.displayName;
            if (promptField.kind == PromptFieldKind::ImageButton)
                std::cout << " (image path: PNG/JPEG/TIFF)";
            if (!existingValue.empty())
                std::cout << " [" << existingValue << "]";
            if (promptField.kind == PromptFieldKind::Value && promptField.widget)
            {
                if (auto onState = getOnStateName(promptField.widget); !onState.empty())
                    std::cout << " (on value: " << onState << ")";
            }
            std::cout << ": ";

            std::string input;
            std::getline(std::cin, input);
            if (input.empty())
                continue;

            if (promptField.kind == PromptFieldKind::ImageButton)
            {
                try
                {
                    setImageButtonAppearanceFromPath(mako, promptField.widget, input);
                }
                catch (IError& e)
                {
                    const String errorFormatString = getEDLErrorString(e.getErrorCode());
                    std::wcerr << L"Warning: could not load image for " << promptField.displayName.c_str()
                        << L": " << e.getErrorDescription(errorFormatString) << L'\n';
                }
                catch (std::exception& e)
                {
                    std::wcerr << L"Warning: could not load image for " << promptField.displayName.c_str()
                        << L": " << e.what() << L'\n';
                }
            }
            else if (promptField.field)
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
                if (isImageButtonWidget(form, widget))
                    continue;

                try
                {
                    const auto style = getDefaultAppearanceStyle(mako, form, widget);
                    if (style.usedFallback && shouldWarnForDefaultAppearanceFallback(form, widget))
                    {
                        std::wcerr << L"Warning: " << widgetQualifiedName(form, widget).c_str()
                            << L": " << style.warning.c_str() << L'\n';
                    }
                    widget->createBasicAppearances(form, getOnStateName(widget).c_str(), style.font, style.fontIndex, style.fontSize);
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
            << L"For image button fields, enter a PNG, JPEG, or TIFF path; the image loader itself accepts IInputStreamPtr.\n"
            << L"If input.pdf is omitted, the sample first creates FormTemplate.pdf with fields named\n"
            << L"CustomerName, JobNumber, Approved, and ImportedImage, then prompts for values and renders that template.\n";
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

        promptForFieldValues(mako, document);
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
