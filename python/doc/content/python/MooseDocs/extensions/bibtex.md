# Bibtex Extension

The `bibtex` extension allows you to cite references stored in BibTeX database(s).

## Configuration

The `bibtex` extension is enabled by default. The only thing necessary for using
the citation commands is to provide properly-formatted `.bib` file(s) within the
included content in your configuration file.

## Usage

Two commands are provided for citing BibTeX references: `!cite` and `!citep`.
The latter includes parentheses, while the former does not. To cite multiple
references, they are separated with a comma.

To generate the list of references cited on a page, the `!bibtex bibliography`
command is used. If it is not used, then it will automatically be placed at the
bottom of the page.

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
