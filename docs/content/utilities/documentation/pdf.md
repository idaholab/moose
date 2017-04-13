name:PDF

# Generating PDFs

The MOOSE documentation system is capable of creating latex (*.tex) files or PDFs from
[MOOSE flavored markdown](moose_flavored_markdown.md).

## Creating Content

Creating a pdf begins by creating a markdown file with the desired content. For example, the file `~/projects/moose/docs/examples/report.md` contained in MOOSE is a report containing all the
information regarding the MOOSE documentation system, including this page, to demonstrate the capability and syntax.

!text docs/examples/report.md max-height=500px

Notice that this markdown file has some special syntax located at the top. The PDF creation
process involes generating a tex file, the file generated is based on [Jinja2](http://jinja.pocoo.org) template. The top commands are used to set template parameters from the markdown file.

However, it is also possible to set these paramaters from the command-line as well, please refer
to the `moosedocs.py latex --help` output for further details regarding the template paramters.


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
