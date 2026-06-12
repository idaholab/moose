# Bibtex Extension

The `bibtex` extension allows you to cite references stored in BibTeX database(s).

## Configuration

The `bibtex` extension is enabled by default. The only thing necessary for using
the citation commands is to provide properly-formatted `.bib` file(s) within the
included content in your configuration file.

The inline citation style is controlled by the `citation_style` configuration
option, which applies to every page:

- `author-year` (default): inline citations render as author and year, e.g.
  "Slaughter et al. (2014)".
- `number`: inline citations render as bracketed numbers, numbered in the order
  they are first cited. The reference list is ordered to match, so `[1]` always
  points to the first entry. This is useful on pages that cite many works by the
  same authors or year, where author-year labels are hard to tell apart.

```yaml
Extensions:
    MooseDocs.extensions.bibtex:
        citation_style: number
```

## Usage

Two commands are provided for citing BibTeX references: `!cite` and `!citep`.
`!cite` is textual, so the citation reads as part of the sentence, while `!citep`
is parenthetical, for use inside or at the end of a sentence. How each renders
depends on the `citation_style` configured above:

- With `author-year`, `!cite` reads as "Slaughter et al. (2014)" and `!citep`
  wraps the author and year in parentheses, e.g. "(Slaughter et al., 2014)".
- With `number`, `!citep` renders just the bracketed number, e.g. `[1]`, while
  `!cite` keeps the author so it still reads in running text, e.g.
  "Slaughter et al. `[1]`" (a bare number would not read well there).

To cite multiple references, they are separated with a comma.

To generate the list of references cited on a page, the `!bibtex bibliography`
command is used. If it is not used, then it will automatically be placed at the
bottom of the page.

Each entry in the reference list provides an `[Export]` link that opens the
citation in three formats: BibTeX, RIS (for import into Microsoft Word, EndNote,
Zotero, or Mendeley), and a plain-text reference string. The RIS and plain-text
formats are intended for those who do not write in LaTeX.

The examples in [citation_examples] use the default `author-year` style, which is
the style used throughout the MOOSE documentation website.

!devel! example caption=Citation examples using the default author-year style. id=citation_examples
[!cite](slaughter2014framework)

[!cite](slaughter2014framework, slaughter2015continuous)

[!cite](slaughter2014framework, slaughter2015continuous, gaston2015physics)

[!citep](slaughter2014framework)

[!citep](slaughter2014framework, slaughter2015continuous)

!bibtex bibliography

Some text after the references.
!devel-end!
