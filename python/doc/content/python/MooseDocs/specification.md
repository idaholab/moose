# MOOSE Markdown Specification (MooseDown)

The following details the MOOSE flavored [markdown] used for documenting MOOSE and
MOOSE-based applications with the MooseDocs system.

## Motivation id=motivation

As a short-hand [HTML] format, [markdown] is ubiquitous, especially among software developers.
However, no standard exists and the original implementation was incomplete. Currently, there are
myriad implementations---often deemed "flavors"---of Markdown. [CommonMark](http://commonmark.org/)
is a proposed standard. However, this specification is syntactically loose. For example, when
defining lists the spacing can be misleading;
[Example 273](http://spec.commonmark.org/0.28/#example-273) and
[Example 268](http://spec.commonmark.org/0.28/#example-268) shows that some poorly defined behavior
still exists. It is stated in the specification that the associated rule "should prevent most spurious list captures,"
which is inadequate for a language.

Additionally, most parsers of this specification do not support custom extensions or adding them is
difficult, from a user perspective, because the parsing strategy used is complex and context
dependent.

Originally, MooseDocs used the [markdown](http://pythonhosted.org/Markdown/) python package, which
worked well in the general sense. However, as the MooseDocs system matured a few short-comings were
found. The main problems, with respect to MooseDocs, was the parsing speed, the lack of an [AST],
and the complexity of adding extensions (i.e., there are too many extension formats). The lack of an
[AST] limited the ability to convert the supplied markdown to other formats (e.g., LaTeX).

For these reasons, among others not mentioned here, yet another markdown flavor was born, MOOSE
flavored Markdown ("MooseDown"). The so called MooseDown language is designed to be strict with
respect to format as well as easily extendable so that MOOSE-based applications can customize the
system to meet their documentation needs. The strictness allows for a simple parsing strategy to be
used and promotes uniformity among the MooseDown files.

## MooseDocs Extensions

The MooseDocs systems work using extensions, thus allowing for arbitrary languages and syntax
to be supported. The MooseDown language is defined using the extensions listed in the following
tables. [user-ext] provides a list of extensions that are useful for those writing documentation and
[devel-ext] includes extension information for developers of new extensions.


!table id=user-ext caption=List of extensions useful for writing "MooseDown".
| Extension | Description |
| :- | :- |
| [extensions/core.md] | Basic markdown syntax such as code blocks, lists, and bold text. |
| [extensions/autolink.md] | Automatic linking across pages within markdown. |
| [extensions/table.md] | Provides means for implementing tables using traditional markdown format. |
| [extensions/katex.md] | Enables use of KaTeX rendered equations. |
| [extensions/bibtex.md] | Enables use of BibTeX citations and bibliographies. |
| [extensions/common.md] | Defines a means for defining global shortcut syntax. |
| [extensions/listing.md] | Provides commands for including source code directly from the repository. |
| [extensions/include.md] | Allows for markdown files to be included in other markdown files. |
| [extensions/alert.md] | Creates alert boxes to draw attention to content. |
| [extensions/media.md] | Extension for including images and movies. |
| [extensions/appsyntax.md] | Enables the use of MOOSE application syntax within markdown files. |
| [extensions/sqa.md] | Provides tools for writing software quality documentation using templates. |
| [extensions/layout.md] | Provides tools for creating columns and tabs via markdown. |
| [extensions/acronym.md] | Provides means for defining and listing acronyms across pages. |
| [extensions/graph.md] | Adds [plotly](https://plot.ly) support for creating charts. |
| [extensions/gallery.md] | Tools for building image galleries. |
| [extensions/style.md] | Command for setting text styling. |
| [extensions/ifelse.md] | Commands for if/elif/else statements in markdown. |
| [extensions/template.md] | Tools for building MooseDocs markdown templates. |
| [extensions/datetime.md] | Functions for adding date/time information. |
| [extensions/gitutils.md] | Tools for gleaning information regarding git repository. |
| [extensions/algorithm.md] | Provides means of displaying algorithms. |
| [extensions/tag.md] | Provides means for tagging markdown pages for documentation filtering. |

!table id=devel-ext caption=List of extensions useful for writing extensions for "MooseDown".
| Extension | Description |
| :- | :- |
| [extensions/command.md] | Basis for creating extensions that rely on commands (see [extensions/appsyntax.md]). |
| [extensions/floats.md] | Tools for creating numbered and/or captioned content (see [extensions/media.md]). |
| [extensions/modal.md] | Creates links to modal windows displaying code content. |
| [extensions/devel.md] | Tools for documenting MooseDocs extensions. |
| [extensions/package.md] | Tools for linking to MOOSE environment packages. |
| [extensions/content.md] | Creates complete list of markdown pages within a directory. |

[AST]: https://en.wikipedia.org/wiki/Abstract_syntax_tree

[HTML]: https://en.wikipedia.org/wiki/HTML

[CommonMark]: http://commonmark.org/

[markdown]: https://en.wikipedia.org/wiki/Markdown
