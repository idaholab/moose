# Documentation Setup and Configuration

!!!warning
    This system is currently experimental, under development, and subject to change. This page is being written as
    the various components become available.

MOOSE contains a single-source documentation system for creating websites, slideshows, and pdfs
using [markdown](https://en.wikipedia.org/wiki/Markdown) syntax, including a custom syntax aimed to simply the
process: [Moose Flavored Markdown](moose_flavored_markdown.md).

---

## Dependencies
If you are using a current MOOSE package then a majority of the setup is complete, only the large file storage system needs
to be installed, which can be done by running the following command:

```bash
git lfs install
```

If you are working on documentation in MOOSE or an application that has existing documentation then
you will need to run the following command from within the repository to download any existing images.

```bash
git lfs pull
```


### Manual Setup
If you are not using a MOOSE package, then the following steps may be performed to install the necessary dependencies
for using the documentation system.

(1) Install Git LFS
Install the latest Git Large File Storage package for your system following the getting started instruction from [git-lfs.github.com/](https://git-lfs.github.com/). If you are working on documentation in MOOSE or an application that has existing documentation then
you will need to run the following command from within the repository to download any existing images.

```bash
git lfs pull
```

(1) Create a new python environment.

This will create a special python environment suitable for using the documentation system, this is done so that an existing
python configuration you have will not be damaged.

```bash
conda create --name docs --clone root
```

(2) Activate the new environment.

This command will need to be executed within any terminal that you plan to perform documentation related tasks.

```bash
source activate docs
```

(3) Install the additional python packages.

```bash
pip install markdown markdown-include python-markdown-math bs4 pybtex jinja2 livereload
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

The "moosedocs.yml" file that was copied will likely need to be modified for your application. This configuration file as
well as all other paths within the documentation are always provided

The following configuration options are available.

| Options | Default | Description |
| ------- | ------- | ----------- |
| site_dir | "site" | Location that the website content will be copied when running the 'build' command.
| navigation | navigation.yml | A yaml file containing the sites top navigation menu, which is limited to one nested level.
| template | materilize.html | Name of the html template file to utilize, it must be located in the 'templates' directory.
| template_arguments | dict() | A dictionary of template arguments that are passed to the template.
| markdown_extensions | [] | A list of the markdown extensions to utilize when building the site.

The MOOSE documentation system uses the [python markdown](http://pythonhosted.org/Markdown/) package from which a custom set of markdown
was created, the available options for this package are listed in the table below as well as the configuration for MOOSE documentation.

!text docs/moosedocs.yml max-height=400px overflow-y=scroll

| Option       | Default | Description |
| ------------ | ------- | ----------- |
| executable   |         | The MOOSE application to execute to generate syntax. |
| locations    | dict()  | A list of locations to search for objects and systems that should be documented. |
| repo         |         | The location for the remote repository for creating hyperlinks. |
| links        |         | Source code paths for generating code links. This options could contain headings (e.g., Tests), under each heading is a list of  directories that will be searched for input files and source code. |
| slides       | False   | Enable the parsing for creating reveal.js slides. |
| package      | False   | Enable the use of the MoosePackageParser. |
| graphviz     | '/opt/moose/graphviz/bin' | The location of graphviz executable for use with diagrams. |
| dot_ext      | 'svg'   | The file extension to utilize with graphviz dot execution. |

The 'locations' option contains should contain sub-items, as shown above in the MOOSE configuration file. These sub-items
include:

| Option   | Description |
| -------- | ----------- |
| doxygen  | The path to the doxygen website, used for developer links. |
| paths    | A list of paths to the source code (i.e., source and include directories).|
| install  | The location where the markdown is located for this documentation sub-item. |
| hide     | The MOOSE syntax to ignore. |
