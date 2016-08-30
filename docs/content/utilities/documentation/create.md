<!-- content/doc_generation/Overview.md -->

[mkdocs]: www.mkdocs.org


MOOSE includes tools for auto-generating web-sites (like this one) that documents your application. Included
in this tool is the ability to:

* Add detailed descriptions of custom systems and objects using [Moose Style Markdown](moose_flavored_markdown.md),
which has useful features designed for documenting source code and input file syntax;
* Generate pages based on class descriptions and input parameter documentation included in your source code;
* Generate links to examples and source code that use or inherit from your objects;
* Create summary tables of objects included in your application; and
* Include documentation from the MOOSE framework, modules, or other applications that are registered in your application.

---

## Adding Documentation
There are two primary locations that documentation should be added: Source Code and Markdown Descriptions.

### Source Code
The first step is to add documentation for your application in the `validParams` function. This is done by adding a
and parameter documentation strings and calling the class description method.

A description of each parameter should also be provided when they are added with the various add methods
of the `InputParameters` object. For example, in the [FunctionIC](../systems/framework/ICs/FunctionIC.md)
the following parameter documentation is added, which is then present in the parameter summary table of the
generated site.

!text framework/src/ics/FunctionIC.C line=addRequiredParam

The string supplied in this function will appear in the parameter tables within the documentation that is generated.
For example, see the parameter table for the [DirichletBC object](/BCs/DirichletBC.md).

Secondly, a short description should be supplied in each `addParam`, `addPrivateParam`, etc. function in your code. For
example, in the [Diffusion](/Kernels/Diffusion.md) object the following class
description.

!text framework/src/kernels/Diffusion.C line=addClassDescription

When the documentation for this object is generated this string is added to the first portion of the page and the
system overview table. For example, the [Kernels overview](/Kernels/Overview.md) includes a table with each object
listed; the table includes the class description from the source code.

### Markdown Descriptions
A detailed description may be provided to the generated documentation for an object by creating a markdown file using
[MOOSE Flavored Markdown](moose_flavored_markdown.md). The created file must be stored in a file named according to the
registered MOOSE syntax within the "details" directory explained in the [Configuration](#configuration) section below.
For example, the details for the [Diffision](/Kernels/Diffusion.md) are in the `framework/docs/details/Kernels/Diffusion.md`
file.

---

## Configuration
The documentation generation system for MOOSE is built upon [mkdocs], thus it is configured
using [YAML](http://yaml.org) files. For MOOSE there is a single configuration file that must be created, for applications
the location of this file (`moosedocs.yml`) is normally under the "docs" directory (e.g., `moose/docs`).

In general, there are three settings that should be set within this configuration file.

### app:
The app should contain a single entry that is the directory containing the executable for the application from which
documentation shall be created.

### defaults:
As will be detailed further below, it is possible to include objects from multiple sources. Each of these sources
can be configured individually. Each of these options default to the values listed in this section. The following table
includes the complete set of options that can be set in the defaults section and/or the included sources.

| Option | Description |
| ------ | ----------- |
| details | The location of the detailed object description markdown files (see [Adding Documentation](#adding-documentation)). |
| source | The location of the source code (.C/h files), this is required to get automatic detection of object inheritance. |
| install | The location of the generated markdown to be installed. The page is saved with the input syntax appended (e.g., <install>/Kernels/Diffusion.md). |
| repo | The remote repository (e.g., github or bitbucket) for generating links. |
| doxygen | The generated Doxygen pages for generating links. |
| hide | A list of syntax to ignore. |
| links | Source code paths for generating code links. This options could contain headings (e.g., Tests), under each heading is a list of directories that will be searched for input files and source code. |

### include:
The include option contains the information that indicates what documentation should be generated. For example, within
the MOOSE documentation the following is contained within this option to build the documentation from the framework itself
as well as the phase field module.

!text docs/moosedocs.yml start=include end=contact:

The names provided in the sub-sections (i.e., "framework" and "phase_field") are arbitrary. All of the options listed
in the table above may be used within each of the sub-sections to taylor the generation of the pages as needed to
separate source code directories or include required MOOSE applications.

---

## Site Pages

The final step when preparing an application site is to define the website page layout, this is done in "pages.yml" within
the docs directory. The information within this file mimics the [mkdocs pages](http://www.mkdocs.org/user-guide/configuration/#pages)
configuration, with one exception: it is possible to include other markdown files. For example, the "pages.yml" for
the MOOSE website includes the following.

!text docs/pages.yml

Notice, that the framework and the modules each have include statements pointing to another "pages.yml" file. This
file is generated for each item in the "include" setting discussed above and is placed in the "install" directory.

---

## MkDocs Configuration
As mentioned the MOOSE documentation system relies on [mkdocs], thus a configuration file must exist for it. MOOSE
uses the default filename from [mkdocs]: "mkdocs.yml."  Therefore, any changes that need to be made to [mkdocs] must
be done in this file.

##### **Markdown Extensions**

The [mkdocs] uses the python [markdown](http://pythonhosted.org/Markdown/) package and adds a custom extension to add the
necessary functionality. This custom extension is located in `python/MooseDocs/extensions/MooseMarkdown.py` and includes
three configuration options, which are set as done in the `mkdocs.yml` file.

!text docs/mkdocs.yml start=markdown_extensions

| Option | Description |
| ------ | ----------- |
| root   | The root directory of the repository, if not provided the root is found using git. |
| make   | The location of the Makefile responsible for building the application (default: 'root'). |
| repo   | The location for the remote repository for creating hyperlinks. |

---

## Building Documentation

### Setup
Currently, the documentation system for MOOSE is under development so the required dependencies are not yet
part of the MOOSE environment package, therefore some setup is required.

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

### Generate, Serve, and Deploy
To generate, view, and/or build the documentation perform the following.

(1) Move to your application docs directory (where the [Configure File](#configuration) is located).
```text
cd ~/projects/your_app_name_here/docs
```

(2) Generate the application specific Markdown.

```text
./moosedocs.py generate
```

(3) Serve the page to a local server.

```text
./moosedocs.py serve
```

To deploy the page, run the script with the 'build' options. This will create the site in the location is given
by "site_dir" in your [MkDocs Configure File](#mkdocs-configuration). This site can then be copied to a server for
hosting.

```text
./moosedocs.py build
```
