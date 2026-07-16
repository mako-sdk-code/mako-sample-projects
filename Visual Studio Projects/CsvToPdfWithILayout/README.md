# CSV to PDF with ILayout (C#)

This sample renders a semicolon-delimited CSV file as an A4 landscape PDF table.

`ILayout` supplies Unicode text shaping, wrapping, line layout, and DOM bounds. The small
table layer supplies the behavior that `ILayout` does not model directly: fixed column
widths, repeated headers, row/page breaks, alternating fills, borders, and page footers.

The sample expects these columns, in order:

```text
Id;UserName;Message;TimeStamp;ModuleName;OldValue;NewValue
```

Run it with:

```powershell
dotnet run --project .\CsvToPdfWithILayoutCS\CsvToPdfWithILayoutCS.csproj -- `
  C:\path\to\input.csv `
  C:\path\to\output.pdf
```

To use Consolas for the table, add the monospace flag:

```powershell
dotnet run --project .\CsvToPdfWithILayoutCS\CsvToPdfWithILayoutCS.csproj -- `
  C:\path\to\input.csv `
  C:\path\to\output.pdf `
  --monospace
```

When `--monospace` is present, the sample uses Consolas for all table text. When it is
omitted, the sample uses DejaVu Sans with Arial as a fallback. Headers use the same
regular-weight font as the body.

The column widths and page geometry are constants near the top of
`CsvToPdfWithILayout.cs`. The CSV is parsed with `TextFieldParser`, so quoted delimiters,
escaped quotes, and embedded newlines are preserved.

Rows that fit on a fresh page are kept together. If a single row is taller than the
available page body, it is split only at the line-box boundaries produced by `ILayout`
and continues below the repeated header on the next page.
