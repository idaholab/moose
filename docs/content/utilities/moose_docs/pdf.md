name:'PDF'

# Generating PDFs

The MOOSE documentation system is capable of creating latex (*.tex) files or PDFs from
[MOOSE flavored markdown](moose_markdown/index.md).

!include docs/content/utilities/moose_docs/config.md

For example the file shown below is the pdf/latex configuration file from MOOSE. The most important
portion of this files the the ["MooseDocs.extensions.template"](extensions/templates.md) entry. This provides the template for
resulting html, which in this case is setup for latex output.

!listing docs/latex.yml

## Creating Content

Creating a pdf begins by creating a markdown file with the desired content. For example, the file `~/projects/moose/docs/examples/report.md` contained in MOOSE is a report containing all the
information regarding the MOOSE documentation system, including this page, to demonstrate the capability and syntax.

!listing docs/examples/report.md max-height=500px

Notice that this markdown file has some special syntax located at the top. The PDF creation
process involves generating a tex file. The ["MooseDocs.extensions.template"](extensions/templates.md) extension is based on [Jinja2](http://jinja.pocoo.org) template. The top commands are used to set template parameters from the markdown file.


## Building the PDF

The `~/projects/moose/docs/moosedocs.py' python utility is used for converting the markdown
that was created into a pdf.

```bash
cd ~/projects/moose/docs
./moosedocs latex examples/report.md
```

This will create an report.pdf in the examples directory. Run `./moosedocs latex --help`
for additional options when running this script.

To view the pdf, simply open the report.pdf file created. On OSX you can preform the following.

```bash
open ~/projects/moose/docs/examples/report.pdf
```
