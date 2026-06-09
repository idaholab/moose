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
- `number`: inline citations render as bracketed numbers, e.g. `[1]`, numbered
  in the order they are first cited. The reference list is ordered to match, so
  `[1]` always points to the first entry. This is useful on pages that cite many
  works by the same authors or year, where author-year labels are hard to tell
  apart.

```yaml
Extensions:
    MooseDocs.extensions.bibtex:
        citation_style: number
```

## Usage

Two commands are provided for citing BibTeX references: `!cite` and `!citep`.
The latter includes parentheses, while the former does not. To cite multiple
references, they are separated with a comma.

To generate the list of references cited on a page, the `!bibtex bibliography`
command is used. If it is not used, then it will automatically be placed at the
bottom of the page.

Each entry in the reference list provides an `[Export]` link that opens the
citation in three formats: BibTeX, RIS (for import into Microsoft Word, EndNote,
Zotero, or Mendeley), and a plain-text reference string. The RIS and plain-text
formats are intended for those who do not write in LaTeX.

See [citation_examples] for examples.

!devel! example caption=Citation examples. id=citation_examples
[!cite](slaughter2014framework)

[!cite](slaughter2014framework, slaughter2015continuous)

[!cite](slaughter2014framework, slaughter2015continuous, gaston2015physics)

[!citep](slaughter2014framework)

[!citep](slaughter2014framework, slaughter2015continuous)

!bibtex bibliography

Some text after the references.
!devel-end!
