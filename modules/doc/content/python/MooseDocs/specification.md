# MOOSE Markdown Specification (MooseDown)

The following details the MOOSE flavored [markdown] used for documenting MOOSE and
MOOSE-based applications with the MooseDocs system.

## Motivation id=motivation

As a short-hand [HTML] format, [markdown] is ubiquitous, especially among software developers.
However, no standard exists and the original implementation was incomplete. Currently, there are
myriad implementations---often deemed "flavors"---of Markdown. [CommonMark](http://commonmark.org/)
is a proposed standard. However, this specification is syntactically loose. For example, when
defining lists the spacing is can be misleading, see
[Example 273](http://spec.commonmark.org/0.28/#example-273) and
[Example 268](http://spec.commonmark.org/0.28/#example-268) shows that some poorly defined behavior
still exists and it is stated that the associated rule "should prevent most spurious list captures,"
which is inadequate for a language.

Additionally, most parsers of this specification do not support custom extensions or adding them is
difficulty, from a user perspective, because the parsing strategy used is complex and context
dependent.

Originally, MooseDocs used the [markdown](http://pythonhosted.org/Markdown/) python package, which
worked well in the general sense. However, as the MooseDocs system matured a few short-comings were
found. The main problems, with respect to MooseDocs, was the parsing speed, the lack of an [AST],
and the complexity of adding extensions (i.e., there are too many extension formats). The lack of an
[AST] limited the ability to convert the supplied markdown to other formats (e.g., LaTeX).

For these reasons, among others not mentioned here, yet another markdown flavor was born. MOOSE
flavored Markdown ("MooseDown"). The so called MooseDown language is designed to be strict with
respect to format as well as easily extendable so that MOOSE-based applications can customize the
system to meet their documentation needs. The strictness allows for a simple parsing strategy to be
used and promotes uniformity among the MooseDown files.

## MooseDocs Extensions

The MooseDocs systems works using extensions, thus allowing for arbitrary languages and syntax
to be supported. The MooseDown language is defined using the extensions listed in the following
tables. [user-ext] provides a list of extensions that are useful for those writing documentation and
[devel-ext] include extension information for developers for new extensions.


!table id=user-ext caption=List of extensions useful for writing "MooseDown".
| Extension | Description |
| :- | :- |
| [/core.md] | Basic markdown syntax such as code blocks, lists, and bold text. |
| [/autolink.md] | Automatic linking across pages within markdown. |
| [/table.md] | Provides means for implementing tables using traditional markdown format. |
| [/katex.md] | Enables use of KaTeX rendered equations. |
| [/bibtex.md] | Enables use of BibTeX citations and bibliographies. |
| [/common.md] | Defines a means for defining global shortcut syntax. |
| [/listing.md] | Provides commands for including source code directly from the repository. |
| [/include.md] | Allows for markdown files to be included in other markdown files. |
| [/alert.md] | Creates alert boxes to draw attention to content. |
| [/media.md] | Extension for including images and movies. |
| [/appsyntax.md] | Enables the use of MOOSE application syntax within markdown files. |
| [/sqa.md] | Provides tools for writing software quality documentation using templates. |
| [/layout.md] | Provides tools for creating columns and tabs via markdown. |
| [/acronym.md] | Provides means for defining and listing acronyms across pages. |
| [/plotly.md] | Adds [plotly](https://plot.ly) support for creating charts. |
| [/gallery.md] | Tools for building image galleries. |

!table id=devel-ext caption=List of extensions useful for writing extensions for "MooseDown".
| Extension | Description |
| :- | :- |
| [/command.md] | Basis for creating extensions that rely on commands (see [/appsyntax.md]). |
| [/floats.md] | Tools for creating numbered and/or captioned content (see [/media.md]). |
| [/devel.md] | Tools for documenting MooseDocs extensions. |
| [/package.md] | Tools for linking to MOOSE environment packages. |
| [/contents.md] | Creates complete list of markdown pages within a directory. |

[AST]: https://en.wikipedia.org/wiki/Abstract_syntax_tree

[HTML]: https://en.wikipedia.org/wiki/HTML

[CommonMark]: http://commonmark.org/

[markdown]: https://en.wikipedia.org/wiki/Markdown
