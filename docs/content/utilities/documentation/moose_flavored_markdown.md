# MOOSE Flavored Markdown

Documentation generated using MOOSE is generated using the [python-markdown](http://pythonhosted.org/Markdown/) package,
which includes the ability to use extensions from others as well as define custom extensions. This page outlines the
extensions included as well as the custom syntax defined exclusively for documenting MOOSE source code.

---

## Extensions
### Symbol Conversion
This package converts ASCII symbols for dashes, quotes, and ellipses to the correct html, for more information see the
documentation for this package: [SmartyPants](http://pythonhosted.org/Markdown/extensions/smarty.html).

### Markdown Include
This package allows for other markdown file to be include within the current file by enclosing the markdown file to
be included within \{\! and \!\}. For more information refer to the package documentation: [markdown-include](https://github.com/cmacmackin/markdown-include).

### Mathematics
The enables the use of [MathJax](http://www.mathjax.org) within markdown, refer to the package documentation for complete
details: [python-markdown-math](https://github.com/mitya57/python-markdown-math).

Inline math may be specified by enclosing the latex in single `$`: $y=a\cdot x + b$. Additionally, stand-alone math may
be enclosed in `$$`:

$$
x=\frac{1+y}{1+2z^2}.
$$

### Admonition
The [admonition](https://pythonhosted.org/Markdown/extensions/admonition.html) package enables for important and critical
items to be highlighted, using the syntax detailed below and the package documentation: [admonition](https://pythonhosted.org/Markdown/extensions/admonition.html).

```markdown
!!! type "An optional title"
    A detailed message paragraph that is indented by 4 spaces and can include any number of lines.
```

The supported "types" for MOOSE are: "info", "note", "important, "warning", "danger", and "error."

!!! info "Optional Info Title"
    This is some information you want people to know about.

!!! note "Optional Note Title"
    This is an example of a note.

!!! important "Optional Important Title"
    This is an example of something important.

!!! warning "Optional Warning Title"
    This is a warning.

!!! danger "Optional Danger Title"
    This is something very dangerous.

!!! error "Optional Error Title"
    This is an error message.

---

## Automatic Links

Moose Flavored Markdown is capable of automatically creating links based on Markdown filenames, which is
especially useful when linking to generated pages. The syntax is identical to creating links as
defined by [mkdocs], however the markdown path may be incomplete.

* `[/Diffusion.md]`: [/Diffusion.md]
* `[/Kernels/index..md]`: [/Kernels/index.md]
* `[Diffusion](/Diffusion.md)`: [Diffusion](/Diffusion.md)

---

## Including MOOSE Source Files
It is possible to include complete or partial C++ or input files from the local MOOSE repository. The following sections detail the custom
markdown syntax to needed, including the application of special settings in the form of key, value pairings that are supplied within
the custom markdown. A complete list of available settings is provided in the [Settings](MooseFlavoredMarkdown.md#optional-settings) of the included code.

!!! note
    When including code the path specified should be defined from the "root" directory, which by default is the
    top level of the git repository (e.g., ~/projects/moose).

### Complete Files
You can include complete files from the repository using the `!text` syntax. For example, the following
includes the complete code as shown.

```markdown
!text framework/src/kernels/Diffusion.C max-height=200px strip-extra-newlines=True overflow-y=scroll
```

!text framework/src/kernels/Diffusion.C max-height=200px strip-extra-newlines=True overflow-y=scroll

### Single Line Match
It is possible to show a single line of a file by a snippet that allows the line to be located within
the file. If multiple matches occur only the first match will be shown. For example, the call to
`addClassDescription` can be shown by adding the following.

```markdown
!text framework/src/kernels/Diffusion.C line=addClassDescription
```

!text framework/src/kernels/Diffusion.C line=addClassDescription

### Range Line match
Code starting and ending on lines containing a string is also possible by using the 'start' and 'end'
options. If 'start' is omitted then the snippet will start at the beginning of the file. Similarly, if 'end'
is omitted the snippet will include the remainder of the file content.

```markdown
!text test/tests/kernels/simple_diffusion/simple_diffusion.i start=Kernels end=Executioner overflow-y=scroll max-height=500px
```

!text test/tests/kernels/simple_diffusion/simple_diffusion.i start=Kernels end=Executioner overflow-y=scroll max-height=500px

### Class Methods
By including a method name, in C++ syntax fashion, it is possible to include specific methods from C++ classes in MOOSE. For example,
the following limits the included code to the `computeQpResidual` method.

```markdown
!clang framework/src/kernels/Diffusion.C method=computeQpResidual
```

<!--
!clang framework/src/kernels/Diffusion.C method=computeQpResidual
-->

!!! warning "Warning"
    This method uses the clang parser directly, which can be slow. Thus, in general source code should be
    included using the line and range match methods above and this method reserved for cases where those methods
    fail to capture the necessary code.


### Input File Block
By including a block name the included content will be limited to the content matching the supplied name. Notice that the supplied name may be approximate; however, if it is not unique only the first match will appear.

```markdown
!input test/tests/kernels/simple_diffusion/simple_diffusion.i block=Kernels
```

!input test/tests/kernels/simple_diffusion/simple_diffusion.i block=Kernels

### Optional Settings
The following options may be passed to control how the output is formatted.


| Option               | Default | Description |
| -------------------- | ------- | ----------- |
| strip_header         | True    | Toggles the removal of the MOOSE copyright header. |
| repo_link            | True    | Include a link to the source code on GitHub ("label" must be True). |
| label                | True    | Include a label with the filename before the code content block. |
| overflow-y           | Scroll  | The action to take when the text overflow the html container (see [overflow-y](http://www.w3schools.com/cssref/css3_pr_overflow-y.asp)). |
| max-height           | 500px   | The maximum height of the code window (see [max-height](http://www.w3schools.com/cssref/pr_dim_max-height.asp)). |
| strip-extra-newlines | False   | Remove excessive newlines from the included code. |

---

## MOOSE Syntax
A set of special keywords exist for creating MOOSE specific links and tables within your markdown, each are explained below. Note, the
examples below refer to documentation associated with Kernels and/or the Diffusion Kernel. This should be replaced by
the syntax for the system or object being documented.

* `!description /Kernels/Diffusion`: Inserts the class description (added via `addClassDescription` method) from the compiled application.
* `!parameters /Kernels/Diffusion`: Inserts tables describing the available input parameters for an object or action.
* `!inputfiles /Kernels/Diffusion`: Creates a list of input files that use the object or action.
* `!childobjects /Kernels/Diffusion`: Create a list of objects that inherit from the supplied object.
* `!devel /Kernels/Diffusion`: Creates links to the repository source code and Doxygen page for the object.
* `!subobjects /Kernels`: Creates a table of objects within the supplied system.
* `!subsystems /Adaptivity`: Creates a table of sub-systems within the supplied system.

---

## Images and Slideshows
!image docs/media/memory_logger-plot_multi.png width=30% padding-left=20px float=right caption=The [memory_logger](/memory_logger.md) is a utility that allows the user to track the memory use of a simulation.

It is possible to include images and slideshows of images with more flexibility than standard markdown.

### Single Images
The markdown keyword for MOOSE images is `!image` followed by the filename as shown below. This command, like most of the other
special MOOSE markdown commands except arbitrary html attributes. Therefore, any keyword, value pairs (e.g., `width=50%`) are
automatically applied to the `<figure>` tag of the image. For example, the following syntax was used to include the image on the right.

```markdown
!image docs/media/memory_logger-plot_multi.png width=30% padding-left=20px float=right caption=The [memory_logger](/memory_logger.md) is a utility that allows the user to track the memory use of a simulation.
```

### Slideshows
A sequence of images can be shown via a `carousel`. By default the images will auto cycle between images.

A simple example:

```markdown
!slideshow
    intro.png
    other*.png
```

This would create a slideshow with the first image as `intro.png` and the next images those that are matched by the wildcard `other*.png`.

Valid options for the slideshow are the same as for the `bootstrap` [carousel](http://getbootstrap.com/javascript/#carousel):

| Option               | Default | Description |
| -------------------- | ------- | ----------- |
| interval             | 5000    | The amount of time delay between images, in milliseconds. |
| pause                | hover   | If set to "hover" then the carousel will pause when the mouse is moved over it. |
| wrap                 | true    | If true then the carousel will cycle continuously. |
| keyboard             | true    | If true then the carousel will respond to keyboard events. |

Additionally, a `caption` option can be set globally or for each image line. The global caption will be used if no caption is specified on the image
line.

A full slideshow example might be:
```markdown
!slideshow caption=My caption with spaces interval=5000 pause=null wrap=false keyboard=false width=500px
    docs/media/memory_logger-plot_multi.png caption=Memory Logger plotting two results
    docs/media/memory_logger-darkmode.png caption=Memory Logger utilizing darkmode
    docs/media/memory_*.png
```

!slideshow caption=My caption with spaces interval=5000 pause=null wrap=false keyboard=false width=500px
    docs/media/memory_logger-plot_multi.png caption=Memory Logger plotting two results
    docs/media/memory_logger-darkmode.png caption=Memory Logger utilizing darkmode
    docs/media/memory_*.png

---

## CSS Options

### In-line CSS
!text test/tests/kernels/simple_diffusion/simple_diffusion.i start=Kernels end=Executioner float=right padding-left=20px font-size=smaller

You can provide any valid CSS attribute to any markdown extension (!text, !input, !clang, !image, !slideshow). Some extensions can not be controlled as much as others. For example the !slideshow extension ignores alignment attributes. Your milage may vary.

Some of the most useful ones are perhaps width, float, align and padding. However, it is CSS. So be creative!

In the following example, we will display an input code block. It will 'float' to the right, which should allow this section of text and elements, to appear to the left of the code block, and wrap around it.

  * Keep in mind, that the actual element tag was placed in this document just beneath the "In-line CSS" title. This is because the placement of the element still applies to where it starts being drawn on the screen.

!!! Info
    It should also work with admonitions

Markdown allowing an input code block to float to the right:
```markdown
!text test/tests/kernels/simple_diffusion/simple_diffusion.i start=Kernels end=Executioner float=right padding-left=20px font-size=smaller
```

### Block CSS Options
You can apply a style sheet to a markdown paragraph through the use of !css extension:

```markdown
!css font-size=smaller margin-left=70% color=red text-shadow=1px 1px 1px rgba(0,0,0,.4)
This paragraph should be of a smaller red font with a black offset text shadow. The text
will be aligned to the right due to the margin-left attribute (nice trick to preserve the
'justify' attribute currently in use)

An empty new line, designates the end of the css block.

!css font-size=smaller margin-left=70% color=red text-shadow=1px 1px 1px rgba(0,0,0,.4)
Another paragraph modified by CSS.
```

!css font-size=smaller margin-left=70% color=red text-shadow=1px 1px 1px rgba(0,0,0,.4)
This paragraph should be of a smaller red font with a black offset text shadow. The text
will be aligned to the right due to the margin-left attribute (nice trick to preserve the
'justify' attribute currently in use)

An empty new line, designates the end of the css block.

!css font-size=smaller margin-left=70% color=red text-shadow=1px 1px 1px rgba(0,0,0,.4)
Another paragraph modified by CSS.

---

## Flow Charts
The ability to include diagrams using [GraphViz](http://www.graphviz.org/) using the [dot]() language is provided.
Simply, include the "dot" syntax in the markdown, being sure to include the keywords ("graph" or
"digraph") on the start of a new line.

* The official page for the dot language is detailed here: [dot](http://www.graphviz.org/content/dot-language)
* There are many sites that provide examples, for example:
    * [https://en.wikipedia.org/wiki/DOT_(graph_description_language)](https://en.wikipedia.org/wiki/DOT_(graph_description_language))
    * [http://graphs.grevian.org/example](http://graphs.grevian.org/example)
* There also exists live, online tools for writing dot:
    * [http://dreampuf.github.io/GraphvizOnline/](http://dreampuf.github.io/GraphvizOnline/)
    * [http://www.webgraphviz.com/](http://www.webgraphviz.com/)

For example, the following dot syntax placed directly in the markdown produces the following graph.
```text
graph {
    bgcolor="#ffffff00" // transparent background
    a -- b -- c;
    b -- d;
}
```

graph {
    bgcolor="#ffffff00" // transparent background
    a -- b -- c;
    b -- d;
}

---

## Build Status
!buildstatus https://moosebuild.org/mooseframework/ float=right padding-left=10px

You can add a Civet build status widget to any page using !buildstatus http://url/to/civet

Currently this will only work with Civet CI services.

```markdown
!buildstatus https://moosebuild.org/mooseframework/ float=right padding-left=10px
```
!!! note
    Be sure to follow your !buildstatus extension with an empty new line.


---

## Bibliographies

It is possible to include citations using latex commands, the following commands are supported within the markdown.

* `\cite{slaughter2015continuous}`: \cite{slaughter2015continuous}
* `\citet{wang2014diffusion}`: \citet{wang2014diffusion}
* `\citep{gaston2015physics}`: \citep{gaston2015physics}

The bibliography style may be set within a page using the latex command
`\bibliographystyle{unsrt}`. Three styles are currently available: 'unsrt', 'plain', 'alpha', and 'unsrtalpha'.

The references are displayed by using the latex `\bibliography{docs/bib/moose.bib}` command. This command accepts a comma separated list of bibtex files (*.bib) to use to build citations and references. The files specified in this list must be given as a relative path to the root directory (e.g., `~/projects/moose`) of the repository.

\bibliographystyle{unsrt}
\bibliography{docs/bib/moose.bib}
