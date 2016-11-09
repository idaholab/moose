# Documentation Setup and Configuration

!!! warning
    This system is currently experimental, under development, and subject to change. This page is being written as
    the various components become available.

MOOSE contains a single-source documentation system for creating websites, slideshows, and pdfs (coming soon)
using [markdown](https://en.wikipedia.org/wiki/Markdown) syntax, including a custom syntax aimed to simply the
process: [Moose Flavored Markdown](moose_flavored_markdown.md).

---

## Dependencies
If you are using a current MOOSE package then a majority of the setup is complete. Simply install Git LFS (Large File
storage) as detailed below and you are ready.

### Git LFS
Install the latest Git Large File Storage package for your system following the getting started instruction from [git-lfs.github.com/](https://git-lfs.github.com/). If you are working on documentation in MOOSE or an application that has existing documentation then
you will need to run the following command from within the repository to download any existing images.

```
git lfs pull
```

### Manual Setup
If you are not using a MOOSE package, then the following steps may be performed to install the necessary dependencies
for using the documentation system.

(1) Create a new python environment.

This will create a special python environment suitable for using the documentation system, this is done so that an existing
python configuration you have will not be damaged.

```text
conda create --name docs --clone root
```

(2) Activate the new environment.

This command will need to be executed within any terminal that you plan to perform documentation related tasks.

```text
source activate docs
```

(3) Install the additional python packages.

```text
pip install markdown-include python-markdown-math mkdocs-bootstrap mkdocs-bootswatch beautifulsoup4 pybtex
```

(4) Install development version of [mkdocs](http://www.mkdocs.org/).

Currently, MOOSE requires the development version of [mkdocs](http://www.mkdocs.org/). To install the following must be performed.

```text
cd ~/projects
git clone https://github.com/mkdocs/mkdocs.git
cd mkdocs
python setup.py install
```

## Configuration
If you are adding documentation to an existing project then you will need to add a few files to configure the
documentation system to work properly for your application.

First, you need the main executable ("moosedocs.py") within your application. This is done by copying the file from MOOSE:

```bash
cp ~/projects/moose/docs/moosedocs.py ~/projects/your_application_name/doc
```

Second, you need to create a configuration file. Again, it is best to start by copying the file in MOOSE:

```bash
cp ~/projects/moose/docs/moosedocs.yml ~/projects/your_application_name/doc
```

The "moosedocs.yml" file that was copied will need to be modified for your application.

The MOOSE documentation system relies on [mkdocs](http://www.mkdocs.org/), thus a configuration file must exist for it. MOOSE
uses the "moosedocs.yml" file for this purpose. Therefore, any configuration options for [mkdocs](http:://www.mkdocs.org) is simply added
to the "mooosedocs.yml" file. For additional information regarding the configuration options available please refer to [MkDocs Configuration](http://www.mkdocs.org/user-guide/configuration/).

[MkDocs](http://www.mkdocs.org/) uses the python [markdown](http://pythonhosted.org/Markdown/) package and adds a custom extension to add the
necessary functionality. This custom extension is located in `python/MooseDocs/extensions/MooseMarkdown.py` and includes
various configuration options, which are set in the "moosedocs.yml" file. For example, the following settings are utilized in MOOSE.

!text docs/moosedocs.yml start=markdown_extensions

Notice, that one of the extensions listed is the aforementioned custom package provided by MOOSE. This package contains the following options
to configure the MOOSE documentation system for your application.

| Option       | Default | Description |
| ------------ | ------- | ----------- |
| root         |         | The root directory of the repository, if not provided the root is found using git. |
| make         |         | The location of the Makefile responsible for building the application, if not provided the directory from "root" option is used. |
| executable   |         | The MOOSE application to execute to generate syntax. |
| locations    | dict()  | A list of locations to search for objects and systems that should be documented. |
| repo         |         | The location for the remote repository for creating hyperlinks. |
| links        |         | Source code paths for generating code links. This options could contain headings (e.g., Tests), under each heading is a list of  directories that will be searched for input files and source code. |
| docs_dir     | 'docs'  | The location of the documentation directory. |
| markdown_dir | 'docs/content' | The location of the markdown to be used for generating the site. |
| slides       | False   | Enable the parsing for creating reveal.js slides. |
| package      | False   | Enable the use of the MoosePackageParser. |
| graphviz     | '/opt/moose/graphviz/bin' | The location of graphviz executable for use with diagrams. |
| dot_ext      | 'svg'   | The file extension to utilize with graphviz dot exectution. |
| pages        | 'pages.yml' | The the pages file defining the site map. |

The 'locations' option contains should contain sub-items, as shown above in the MOOSE configuration file. These sub-items
include:

| Option   | Description |
| -------- | ----------- |
| doxygen  | The path to the doxygen website, used for developer links. |
| paths    | A list of paths to the source code (i.e., source and include directories).|
| install  | The location where the markdown is located for this documentation sub-item. |
