[mkdocs]: www.mkdocs.org

MOOSE contains a single-source documentation system for creating websites, slideshows, and pdfs (coming soon)
using [markdown](https://en.wikipedia.org/wiki/Markdown) syntax, including a custom syntax aimed to simply the
process: [Moose Flavored Markdown](moose_flavored_markdown.md).

## Dependencies
If you are using a current MOOSE package then a majority of the setup is complete. Simply install Git LFS (Large File
storage) as detailed below and you are ready.

### Git LFS
Install the latest Git Large File Storage package for your system: [git-lfs.github.com/](https://git-lfs.github.com/).
Following the getting started instructions on this page after the package has installed and you are ready.

If you are writing documentation for MOOSE or another application that has existing documentation, then you will likely
need to pull in any existing documents stored with Git LFS using:

```
git lfs pull
```

### Manual Setup
If you are not using a MOOSE package, then the following steps may be performed to install the necessary dependencies
for using the documentation system.

(1) Create a new python environment.

```text
conda create --name docs --clone root
```

(2) Activate the new environment.

```text
source activate docs
```

(3) Install the additional python packages.

```text
pip install markdown-include python-markdown-math mkdocs-bootstrap mkdocs-bootswatch
```

(4) Install development version of [mkdocs].

Currently, MOOSE requires the development version of [mkdocs]. To install the following must be performed.

```text
cd ~/projects
git clone https://github.com/mkdocs/mkdocs.git
cd mkdocs
python setup.py install
```

## Configuration
If you are adding documentation to an existing project then you will need to add a few files to configure the
documentation system to work properly for you application.

### MOOSE Configuration
The documentation generation system for MOOSE is built upon [mkdocs], thus it is configured
using [YAML](http://yaml.org) files. For MOOSE there is a single configuration file that must be created, for applications
the location of this file (`moosedocs.yml`) is normally under the "docs" directory (e.g., `moose/docs`).

In general, there are three settings that should be set within this configuration file.

#### app:
The app should contain a single entry that is the directory containing the executable for the application from which
documentation shall be created.

#### defaults:
As will be detailed further below, it is possible to include objects from multiple sources. Each of these sources
can be configured individually. Each of these options default to the values listed in this section. The following table
includes the complete set of options that can be set in the defaults section and/or the included sources.

| Option | Description |
| ------ | ----------- |
| details | The location of the detailed object description markdown files (see [Adding Documentation](#adding-documentation)). |
| include | The location of the source code (.h files), this is required to get automatic detection of object inheritance. |
| source | The location of the source code (.C files), this is required to get automatic detection of object inheritance. |
| install | The location of the generated markdown to be installed. The page is saved with the input syntax appended (e.g., <install>/Kernels/Diffusion.moose.md). |
| repo | The remote repository (e.g., github or bitbucket) for generating links. |
| doxygen | The generated Doxygen pages for generating links. |
| hide | A list of syntax to ignore. |
| links | Source code paths for generating code links. This options could contain headings (e.g., Tests), under each heading is a list of directories that will be searched for input files and source code. |

#### include and source:
The "include" and "source" option contains the information that indicates what documentation should be generated. For example, within the MOOSE documentation the following is contained within this option to build the documentation from the framework itself as well as the phase field module.

!text docs/moosedocs.yml start=include end=contact:

The names provided in the sub-sections (i.e., "framework" and "phase_field") are arbitrary. All of the options listed
in the table above may be used within each of the sub-sections to taylor the generation of the pages as needed to
separate source code directories or include required MOOSE applications.

### MkDocs Configuration
As mentioned the MOOSE documentation system relies on [mkdocs], thus a configuration file must exist for it. MOOSE
uses the default filename from [mkdocs]: "mkdocs.yml."  Therefore, any changes that need to be made to [mkdocs] must
be done in this file.

The [mkdocs] uses the python [markdown](http://pythonhosted.org/Markdown/) package and adds a custom extension to add the
necessary functionality. This custom extension is located in `python/MooseDocs/extensions/MooseMarkdown.py` and includes
three configuration options, which are set as done in the `mkdocs.yml` file.

!text docs/mkdocs.yml start=markdown_extensions

| Option   | Description |
| -------- | ----------- |
| root     | The root directory of the repository, if not provided the root is found using git. |
| make     | The location of the Makefile responsible for building the application (default: 'root'). |
| repo     | The location for the remote repository for creating hyperlinks. |
| docs_dir | The location of the markdown to be used for generating the site. |
| slides   | Enable the parsing for creating reveal.js slides (default: False). |
| package  | Enable the use of the MoosePackageParser (default: False). |
| graphviz | The location of graphviz executable for use with diagrams. |
