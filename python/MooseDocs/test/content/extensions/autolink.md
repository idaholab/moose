# Autolink Extension

[core.md]

[this is local](core.md)

## Local Headings id=local-headings

[#local-headings]

[this is local](#local-headings)

## Non-local headings with ID

[core.md#ordered-single-level-lists]

[this is non-local](core.md#unordered-single-level-lists)

## Source Filename

[controls/RealFunctionControl.C]

[Some Code](controls/RealFunctionControl.C)

### Source with language

[/test/run_tests language=python]

## Optional Links

[this is optional](not_a_real_file_name.md optional=True)

## Exact Option

[index.md exact=True]

[Home](index.md exact=True)

## Alternative Links

[this is a local alternative](not_a_real_file_name.md alternative=#local-headings)

[this is an alternative](not_a_real_file_name.md alternative=core.md#unordered-single-level-lists)

[this is an exact alternative](not_a_real_file_name.md alternative=extensions/core.md exact=True)

[this is an optional alternative](not_a_real_file_name.md alternative=also_not_real.md optional=True)

[this is an alternative link to Google](not_a_real_file_name.md alternative=https://www.google.com/)
