# Creating Presentations

The documentation system for MOOSE includes the ability to create html based presentations from
[MOOSE flavored markdown](moose_markdown/index.md). The presentations are generated using the [reveal.js](http://lab.hakim.se/reveal-js/) framework.

!include docs/content/utilities/moose_docs/config.md

For example the file shown below is the presentation configuration file from MOOSE. The most important
portion of this files the the "MooseDocs.extensions.template" entry. This provides the template for
resulting html, which in this case is setup for an html presentation.

!listing docs/presentation.yml

## Creating Content

Creating a presentation begins by creating a markdown file with the desired content. For example, the file `~/projects/moose/docs/examples/presentation.md` contained in MOOSE is a simple presentation to demonstrate the capability and syntax.

## Building the Presentation

The `~/projects/moose/docs/moosedocs.py python utility is used for converting the markdown
that was created into a presentation.

```bash
cd ~/projects/moose/docs
./moosedocs presentation examples/presentation.md
```

This will create an index.html file that contains the presentation. Run `./moosedocs presentation --help` for additional options when running this
script.

To view the slides, simply open the index.html file created with a web-browser. On OSX you can preform the following.

```bash
open ~/projects/examples/presentation/index.html
```
