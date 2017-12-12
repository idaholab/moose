# Motivation
As the MOOSE framework continues to develop so does the documentation requirements. To minimize the amount duplicate documentation a presentation building program was developed to convert wiki content from [MOOSE wiki](www.moooseframework.org/wiki) into presentation slides.

The goal is to have one location for all MOOSE related content, the [wiki](www.mooseframework.org/wiki), from which information can be extracted in a MOOSE-like way to generate slides suitable for presentations and tutorials.

[](---)

# Getting Started
Before learning how to generate slides, it is important to understand the basic framework from which this tool was formulated around [Remark](http://remarkjs.com), which is a "A simple, in-browser, Markdown-driven slideshow tool..."

Before continuing please review the [Remark](http://remarkjs.com) demo: [remarkjs.com](http://remarkjs.com).

[](---)

# Creating Suitable Wiki Content

Unfortunately, the markdown syntax utilized by Remark and mooseframework.org are slightly different. The following list details how to formulate mooseframework.org/wiki content in a form suitable for conversion to Remark syntax.

* Slides are separated by `[](---)` [](--)
* Animation steps within a slide are separated by `[](--)` [](--)
* [Remark properties](https://github.com/gnab/remark/wiki/Markdown#slide-properties) are entered as `[](property: value)`. For example, to change the class settings for the current slide place `[](class: center, middle)` within the wiki content for the current slide. [](--)
* Comments, which are viewable in presentation mode (press `p` within the Remark presentation) are created by using `[](??? Enter your comments here)`. [](--)
* In general, all Remark syntax may be entered inside the parentheses as `[](Remark command here)`, the only limitation is that the brackets must be preceded by a space.

[](??? Test the use of comments)

[](---)
# Math Equations
In order to properly convert math equations from the www.mooseframework.org/wiki to Remark slides they must be implemented in a specific way, this is due to the different rendering engines used by the two platforms.

* In-line equations are encapsulated by **double** dollar signs: $$y=\int x dx $$.
* Equation blocks should be encapsulated in **triple** dollar signs and begin on a newline:
$$$ \underbrace{\left(\nabla\psi, k\nabla u \right)}_{Kernel} -
    \underbrace{\langle\psi, k\nabla u\cdot \hat{n} \rangle}_{BoundaryCondition} +
    \underbrace{\left(\psi, \vec{\beta} \cdot \nabla u\right)}_{Kernel} -
    \underbrace{\left(\psi, f\right)}_{Kernel} = 0 $$$

[](---)
# Images
No special wiki syntax is required for including images; however, there is special syntax within the input file to adjust the appearance of the image for inclusion in the presentation.

[image:64 align:right]
    A demonstration image that is sized and aligned from within the presentation input file.

[](---)
# Slide and Wiki Links
It is possible to create links to mooseframework.org wiki content using the headings. For example, `[Math Equations](#math-equations)` result in creating a link to the [Math Equations](#math-equations) page.

When the wiki content is converted to slides, the first heading in the slide, i.e., the heading that follows `[](---)` is converted into the slide name. Thus, internal wiki links such as the [Math Equations](#math-equations) link will continue to operate correctly.


[](---)
# Input File Syntax

Slide shows may be generated from www.mooseframework.org/wiki content using an input file syntax similar to MOOSE itself, the following information details the input file that converts the wiki content for this tutorial into a presentation.

This complete input file for the presentation constructed from this wiki is included in [~/projects/moose/python/PresentationBuilder/examples/example_02.i](https://github.com/idaholab/moose/tree/devel/python/PresentationBuilder/examples/example_02.i).

[](---)
## Adding a Slide Set
It is possible to add any number of slide sets, think of these as Kernels. In this class a single slide set is being added. In this example the `CoverSet` object is being utilized to create a cover page for the entire presentation.

```
[presentation]
  [./cover]
    type = CoverSet
    title = '**PresentationBuilder**<br><br> A tool for generating slides from MOOSE wiki content'
    background-image = 'inl_dark_title.png'
    class = 'middle, cover'
    contents = true
```

* **type** Determines what slide set object to build, in this case the `CoverSet` is used which to build a presentation cover page and table-of-contents.
* **title** The title to place on the title page. Note, the name of this slide is `cover-title`.
* **background-image** Sets the background of all slides in the slide set to use the specified image.
* **contents** Enables the automatically generated table of contents slide. Note, the name of this slide is `cover-contents`.

[](---)
## Customizing Slides
It is possible to customize the appearance of individual slides from within the input file, this is done by include a `[./Slides]` sub-block within a slide set block.

```
    [./Slides]
      [./cover-contents]
        background-image = 'inl_white_slide.png'
	class = 'left, top'
      [../]
    [../]
  [../]
```
In this case, the [Remark property](https://github.com/gnab/remark/wiki/Markdown#slide-properties) for the slide named ["cover-contents"](#cover-contents) is modified to change the alignment and background image.

[](---)
## Wiki Content
To include wiki content the `DjangoWikiSet` is utilized in an additional top-level input file block, as follows.

```
  [./demo]
    type = DjangoWikiSet
    wiki = 'PresentationBuilder'
    background-image = 'inl_white_slide.png'
    contents = true
    title = 'Slides extracted from the mooseframework.org wiki'
```

In this case this wiki page (www.mooseframework.org/wiki/PresentationBuilder) is being utilize for the source of the slide content. The `title` and `contents` parameters create a title and contents slide specific to the slide content for this set of slides.

[](---)
Again, the slides for this set may be modified from the input file. In this case the cover slides background and CSS style are altered, as is the ["input-file-syntax"](#input-file-syntax) slide with was adjusted to center the content vertically and horizontally.

```
    [./Slides]
      [./demo-title]
        class = 'middle, cover'
        background-image = 'inl_dark_title.png'
      [../]
    [../]
```
[](---)
Notice, by default if a new slide does not contain a heading, as in the case for this and the previous slide, the heading from the previous slide is utilized with "(cont.)" append.
 
[](---)
## Customizing Images

Within the wiki the image may be sized for use in the presentation by including special syntax: `[](image:203 width:200px)`. This changes the settings for image 203 and may be placed anywhere on the wiki.

[](image:203 width:200px)
[image:203 align:right]

[](---)
In similar fashion to slide customization, images may also be customized using the `[./Images]` sub-block. These settings overwrite the setting implemented within the wiki.

```
  ...
    [./Images]
      [./64]
        align = 'center'
        width = '500px'
      [../]
    [../]
  [../]
```

In this case, image number 64 is modified to be center aligned with a total width of 500 pixels. The image number is the id reference used by the mooseframework.org wiki to insert images.

[](---)
## Creating Custom CSS
It is also possible to create custom CSS class, this is accomplished by adding blocks to the `[./CSS]` block in the input file.

```
  [./CSS]
    [./remark-code]
      font-size = '14px'
    [../]
    [./cover]
      color = '#ffffff'
      padding-left = '250px'
    [../]
  [../]
[]
```

The `[./remark-code]` block modifies the Remark code block format to utilize a smaller font. The `[./cover]` creates a new class with white text and padding at the left side. This class is used by the [title page](#1).

[](---)
## Creating Columns
[](.left-column[)
It is possible to create column within a slide by using custom CSS and relying on the [Remark content](https://github.com/gnab/remark/wiki/Markdown#content-classes) class support.

To create columns:

* Add the column layout(s) to the CSS block of your input file
* In the markdown add the column breaks, using Remark syntax, but protecting the commands using the syntax described previously [Creating Suitable Wiki Content](#creating-suitable-wiki-content). For, example this slide contains such commands to invoke the column creation.

[](])
[](.right-column[)

```
[./right-column]
  width = '30%'
  float = 'right'
[../]
[./left-column]
   width = '65%'
   float = 'left'
[../]
```
[](])

[](---)
## Scrolling Content and GitHub Code

[](.tiny[)
[Steady.C](https://github.com/idaholab/moose/blob/devel/framework/src/executioners/Steady.C)
[](])

[](---)
## Import Wiki Content
[AuxKernels](/wiki/MooseSystems/AuxKernels)

