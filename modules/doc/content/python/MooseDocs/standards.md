# MooseDocs Standards

The standards outlined on this page are intended to aid in creating and maintaining easy-to-read,
focused, and consistent documentation for the MOOSE and MOOSE module codes.

## Content Creation Standards

Developers submitting new classes to the MOOSE repository should also create a new markdown
documentation files with the same names as the created classes.  A template documentation stub file
is provided as a starting place for new markdown documentation pages created as detailed in
[framework/documenting.md].

### End-User Focused

The intended audience for the markdown documentation pages is an end-user with no or little
development experience.  As such, the markdown documentation page should focus on the theoretical
background and input file usage of the corresponding class, including limitations on usability ranges
for the class.  If the background and description are long, break up the information with multiple
second (`##`) and third (`###`) level headings.  Keep in mind that only the second level headings
will appear in the right-hand sidebar as navigation links.

### Citation Information

When appropriate, include citation information for the models implemented in the class being
documented.  Avoid accidental duplication of the citation information in the documentation text by
listing and immediately citing the source; use only `[!cite](example_reference)` which will render as
the author name and publication year in the documentation page.  If a reference is not currently
included in the appropriate bibtex file, add the reference citation information to the bibtex file.

Detailed information about class inheritance should not be included in the markdown text since this
information is already provided in the Doxygen link provided automatically by the MooseDocs system.

### Page Layout

- Use the actual name of the class as the title for the documentation page.
- Use second level headings (denoted with `##`) to aid in page navigation:

  - Begin the markdown file documentation with the heading `## Description`,
  - Immediately before including the example syntax from an input file, use the heading
    `## Example Input File Syntax`,
  - Begin the reference citation section at the end of the file with the heading `## References`,
  - See the template documentation stub file for placement examples of these headings.
- Use the actual name of the class (`CodeClassName`), without spaces, within the first text
  paragraph under the `## Description` heading.
- If additional blocks in the input file are required to run the class being documented (e.g. an
  AuxKernel requires an AuxVariable), include those additional required blocks under the
  `## Example Input File Syntax` heading after the example input syntax for the class being
  documented.

All documentation should be written with American English spelling conventions.

## Equations Standards

- If explaining terms used in an equation immediately after the equation (e.g. the sentence begins
  with "where the term...."), you must include a new line between the equation and the sentence.

- Use only math options supported by [KaTeX](https://khan.github.io/KaTeX/function-support.html).
- To reference an equation, use `\label` within the equation environment.
- If an equation includes a few or more constant value parameters, use a table to list the constant
  parameter variables and values instead of paragraph or a list.
- Powers and orders of magnitude should be formatted as `$6 \times 10^{-6}$`, not as `$6 \cdot
  10^{-6}$` nor as `$e^{-6}$`.
- Exponential terms should be written as `$exp \left( stuff-in-exponent \right)$` instead of
  `$e^{stuff-in-exponent}$`.
- Tables should be used to give the values of parameters used in equations, especially if three or
  more parameters have constant values.

## Units Standards

- For dimensionless parameters, use the notation `(dimensionless)` instead of `(-)` or `(-/-)`.
- Give units in regular font (not bold) (e.g. `$\mathrm{\mu}$m`, not `$\mu m$` ).

## Table Standards

- Include a caption (`caption=`) to describe contents.
- Also include a label (`id=` for tables) to reference the table within the text.
- All numbers and variables in tables should be in bold for consistency.

## Figure Standards

- Include a caption (`caption=`) to describe contents.
- Also include a label (`id=` for tables) to reference the figure within the text.
- Image files should be added to the appropriate module or framework folder in `doc/content/media/`
  - Large images and movies should be add to the
    [large_media](https://github.com/idaholab/large_media) submodule repository, this submodule is
    located at the top level of the repository (e.g., ~/projects/moose/large_media).
- Set the figure image width in a manner to ensure the wrapped text is readable, a setting of 90-95%
  of the text width or 40-70% of the text width is suggested.
