# ExtractSeparation

This standalone .NET 9 Mako console sample extracts a named color separation from the first page of a PDF and writes it to a new, single-page PDF. The selected ink remains a named spot color in the output rather than being converted to a process color or an unnamed grayscale plate.

The sample has no application-specific dependencies. It restores Mako through the `MakoCore.OEM.Net.Windows` NuGet package and targets x64 because the Mako Windows runtime is native.

## How it works

1. `IRendererTransform.findInks()` discovers the inks used on the first page.
2. `IRendererTransform.inkInfoToColorantInfo()` converts the selected ink's alternate representation to DeviceCMYK.
3. Mako creates a one-colorant `IDOMColorSpaceDeviceN`. When written to PDF, this may be represented as the equivalent PDF `/Separation` color space.
4. `ISeparator` produces the selected grayscale separation plate.
5. The separated content is placed in a luminosity soft mask. An inversion transfer function maps black plate data to opaque and white paper to transparent.
6. The mask is applied to a full-tint rectangle painted with the named spot color.
7. The source page is cloned to retain its page boxes and rotation. Its original content is replaced and unfiltered annotations are removed.

The DeviceCMYK space is used only as the spot color's alternate representation. The output content still uses the named spot color.

## Build

Open `ExtractSeparation.sln` in Visual Studio 2022, select the x64 platform, and build the solution.

## Run

Pass an input PDF, an output PDF, and the name of the separation to extract:

```text
ExtractSeparation.exe input.pdf output.pdf "Spot Red"
```

Separation-name matching is case-insensitive. If a path contains spaces, enclose it in quotation marks.

## Exit codes

| Code | Meaning |
| ---: | --- |
| `0` | The separation was found and the output PDF was written. |
| `1` | The arguments are invalid or the input file does not exist. |
| `2` | The requested separation was not found on the first page. |
| `3` | Mako or the file-writing operation reported an error. |

## Notes and limitations

- Only the first page is processed.
- The output contains only the requested separation.
- The spot name and plate data are retained, but the alternate color is generated from Mako's discovered ink information. This does not copy every original DeviceN attribute or tint-transform object verbatim.
- Vector mode asks `ISeparator` to retain vector and text data where possible. Complex transparency may still require rasterized content in the soft mask.
- The DeviceN color space is intentionally created without a DeviceN `/Process` dictionary. A spot-only DeviceN space has no process components; writing an empty `/Components` array produces an invalid PDF that Acrobat may report as "array length is out of range."
