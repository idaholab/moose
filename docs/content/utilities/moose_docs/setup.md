# MooseDocs Setup

MOOSE contains a single-source documentation system for creating websites, slideshows, and pdfs
using [markdown](https://en.wikipedia.org/wiki/Markdown) syntax, including a custom syntax aimed to simplify the
process: [Moose Flavored Markdown](moose_markdown/index.md).

## Dependencies
If you are using a current [MOOSE package](getting_started/installation/index.md) then the setup is
complete.

## Configuration
If you are adding documentation to an existing project then you will need to add a few files to
configure the documentation system to work properly for your application.


###### (1) Create "docs" Location
Create a "docs" directory, where your documentation-related files will be stored.

!!!info
    Most existing applications will have a "doc" directory -- this can be used if desired, or another
    location can be created. The location and name of this directory is arbitrary.

###### (2) Add MooseDocs Executable
To use MooseDocs, an executable is required -- this main executable is simply copied from the
executable within [MOOSE]:

```bash
cp ~/projects/moose/docs/moosedocs.py ~/projects/your_application_name/docs
```

This executable contains command-line based help, which can be accessed using the "-h" flag as
follows.

```
cd ~/projects/your_application_name/docs
./moosedocs.py -h
```

At this point you are ready to begin creating a configuration file for one of the various forms
of documentation that MooseDocs is capable of producing:

* [Creating a Web-Site](moose_docs/website.md)
* [Building a Presentation](moose_docs/presentation.md)
* [Generating a PDF](moose_docs/pdf.md)


## Manual Setup
If you are not using a MOOSE package, then the following packages must be installed, which can
be done using [pip](https://pip.pypa.io/en/stable/).

```bash
pip install --user markdown python-markdown-math bs4 pybtex jinja2 livereload lxml pylatexenc anytree pandas
```
