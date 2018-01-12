# Code Class Name (with Spaces)
`!syntax description /ClassType/CodeClassName`

## Description
Use the `CodeClassName` as it appears within the code and in the input file within the first paragraph under the description.

Citations for source materials should be made in text with the MooseDocs bibtex extension: \cite{example_reference} (`\cite{example_reference}`) renders as both the author name and the publication year.

If referencing an equation within text, use the abbreviation Eq \eqref{eq:example_equation} (`Eq` `\eqref{eq:example_equation}`).
\begin{equation}
\label{eq:example_equation}
\begin{aligned}
x_{example} & = \chi \cdot \nabla + \gamma \\
\chi & = \frac{1}{c} \cdot \frac{\partial x_{example}}{\partial t}
\end{aligned}
\end{equation}
and begin the next line of text without an empty line after the equation, especially if explaining the meaning of parameters.

Tables, which can be referenced in text as \ref{example_table} (`\ref{example_table}`), should use bold for both numbers and variables.

!table id=example_table caption=An example table demonstrating consistency in numbers and variables for the parameters in Eq \eqref{eq:example_equation}
| Example Parameter | Example Value      | Example Units   |
|-------------------|--------------------|-----------------|
| $c$               | $123.4$            | (dimensionless) |
| $\gamma$          | $6 \times 10^{-2}$ | $\mathrm{\mu}$m |

Tables should be used to give the values of parameters used in equations, especially if three or more parameters have constant values.

!media media/moose_logo_small.png width=50% padding-left=20px float=right id=example_figure caption=An example figure set to 50% of the text width with a right float.

Figures should be set to take either the whole page or around half of the page to allow for proper text wrapping around the image, as shown in \ref{example_figure} (`\ref{example_figure}`).
Image widths of less than 40% could be difficult to read, particulary for detailed graphs.

Image files should be added to the appropriate module or framework folder in `docs/content/media/`. Large images and movies should be add to the [large_media](https://github.com/idaholab/large_media) (`[large_media](https://github.com/idaholab/large_media)`) submodule repository.

## Example Input File Syntax
`!listing path/to/test/file block=location/in/input/file`

If additonal blocks in the input file are required to run the class being documented (e.g. the parameter _eigenstrain_names_ in the tensor mechanics strain material and the eigenstrain materials), include those additional blocks under the Example Input File Syntax heading as well.

Include the following three lines of [Moose Style Markdown](moose_docs/moose_markdown/index.md) (`[Moose Style Markdown](moose_docs/moose_markdown/index.md)`) to automatically generate the headings and tables of input parameters, input files which use the class, and other classes which inherit from this class:

`!syntax parameters /ClassType/CodeClassName`

`!syntax inputs /ClassType/CodeClassName`

`!syntax children /ClassType/CodeClassName`

## References
\bibliographystyle{unsrt}
\bibliography{template_example.bib}
