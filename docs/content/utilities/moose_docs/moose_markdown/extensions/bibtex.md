# MooseDocs BibTeX Extension

The BibTeX file format for storing bibliography information is ubiquitous. Therefore, to enable the
ability to create a common set of BibTeX files that can be used with the markdown to create
citations and bibliographies aids in rapid development of documentation.

The [BibTeX] extension makes it possible to include citations using LaTeX commands. The following commands are supported within the markdown.

* `\cite{slaughter2015continuous}`: \cite{slaughter2015continuous}
* `\citet{wang2014diffusion}`: \citet{wang2014diffusion}
* `\citep{gaston2015physics}`: \citep{gaston2015physics}

The bibliography style may be set within a page using the latex command
`\bibliographystyle{unsrt}`. Three styles are currently available: 'unsrt', 'plain', 'alpha', and 'unsrtalpha'.

The references are displayed by using the latex `\bibliography{docs/bib/moose.bib}` command. This command accepts a comma separated list of bibtex files (*.bib) to use to build citations and references. The files specified in this list must be given as a relative path to the root directory (e.g., `~/projects/moose`) of the repository.

\bibliographystyle{unsrt}
\bibliography{docs/bib/moose.bib}

!extension BibtexExtension
