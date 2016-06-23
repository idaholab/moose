## Summary
MOOSE includes tools for autogenerating web-sites that documents your application. Included
in this tool is the ability to:

* Add detailed descriptions of custom systems and objects using [Moose Style Markdown](MooseFlavoredMarkdown.md),
which has useful features designed for documenting source code and input file syntax;
* Generate pages bassed on class descriptions and input parameter documentation included in your source code;
* Generate links to examples and source code that use or inherit from your objects;
* Create summary tables of objects included in your application; and
* Include documentation from the MOOSE framework, modules, or other applications that are registered in your application.

## Documenting Applications
### (1) Source Code Documentation
The first step is to add documentation for your application in the `validParams` function.

#### Class Description
A short description should be supplied in each `validParams` function in your code. For example, in
the [Diffusion](../systems/framework/Kernels/Diffusion.md) object the following class
description, which is added to the generated documentation as the first sentance of the page.

![](framework/src/kernels/Diffusion.C line=addClassDescription)

#### Parameter Descriptions
A description of each parameter should also be provided when they are added with the various add methods
of the `InputParameters` object. For example, in the [FunctionIC](../systems/framework/ICs/FunctionIC.md)
the following parameter documentation is added, which is then present in the parameter summary table of the
generated site.

![](framework/src/ics/FunctionIC.C line=addRequiredParam)

### (2) Configuration
The documentation generation system for MOOSE is built upon [mkdocs](www.mkdocs.org), thus it is configured
using [YAML](http://yaml.org) files. For MOOSE there are two configuration files to setup: a source and application
level configuration.

#### Source Configure File
A "config.yml" file ust exist in each directory containg source code. In general, most
application will contain a single "config.yml" file and it will exist at the top level. However, if your applications
has a more complex structure (e.g., MOOSE itself has the framework and modules directories) multiple configuration
files will allow the documenation to be separated---as in MOOSE---into multiple directories.

A typical configuration file will look similar to those of the modules, for example the following is
the configuation for the Phase Field module.

![](modules/phase_field/config.yml)

The complete list of options and their purpose are as follows.

| Option | Description |
| ------ | ----------- |
| install | The location where the generated markdown will be installed, this defaults to the "install" setting from the applcation configuration file (see [Application Configuration File](Overview.md#application-configuration-file)). |
| details | The location of the object and system detailed description markdown files (see [Detailed Descriptions](#(3)-detailed-descriptions)). |
| prefix | The prefix that is added to the installation directory (see "install"), this allows documentation to be directed to different directories as done with the MOOSE framework documentation. |
| repo | The location of the remote repository where links to source files are directed. This url should be complete to the point where the filename can be append (see the Phase Field configuration above). |
| doxygen | The location of the Doxygen repository. If this is not provided Doxygen links will not be created. |
| hide | A list of syntax to omit from documentation. The list added here is appended to the list provided in the application configuration file. |
| links | A dict, with each entry containing a list of directories to search for usage of the syntax in input and source files.


#### Application Configuration Files

The application level configuration file is used for a large range of tasks, the most relavant shall be explained here. Notice, that this file is used by [mkdocs] as the configuration file, so it contains content not detailed here.

##### **Markdown Extensions**

The [mkdocs] uses the python [markdown](http://pythonhosted.org/Markdown/) package and adds a custom extension to add the
necessary functionality. This custom extension is located in `python/MooseDocs/extensions/MooseMarkdown.py` and includes
three configuration options, which are set as done in the `moosedocs.yml` file.
![](docs/moosedocs.yml start=markdown_extensions: end=pages:)

| Option | Description |
| ------ | ----------- |
| root   | The root directory of the repository, if not provided the root is found using git. |
| make   | The location of the Makefile responsible for building the application (default: 'root'). |
| repo   | The location for the remote repository for creating hyperlinks. |

##### **Pages**
The complete website layout is included in the [`pages`](http://www.mkdocs.org/user-guide/configuration/#documentation-layout) section of the application configuration file. For example, the following is the complete web-site for the MOOSE framework.

![](docs/moosedocs.yml start=pages: end=extra:)

Notice, that these sections include additional files, as shown below. Each of these `pages.yml` files are generated by the directories listed in the `include` option in the following section, which is where the aforementioned `config.yml` files reside (see [Source Configure File](Overview.md#source-configure-file)).

![](docs/moosedocs.yml line=Framework:)

##### **Extra**
The `extra` section of the application configure file requires three items be set, in additional to an optional fourth.

| Option | Description |
| ------ | ----------- |
| app    | (Required) The directory containing the executable for the application. |
| include | (Required) The list of directories to include, these must contain a `config.yml` file. |
| install | (Required) The location to install the generated markdown, the `prefix` set in the [Source Configure File](Overview.md#source-configure-file)
 is append to this location. |
| hide | (Optional) A list of application syntax to ignore when generating documentation. |

For example, the MOOSE framework configuration looks as follows.

![](docs/moosedocs.yml start=extra:)

### (3) Detailed Descriptions
It is possible to add a detailed description, using [Moose Flavored Markdown](MooseFlavoredMarkdown.md) by
creating markdown files that follow an naming convetion associated with the registerd input file syntax and placing these
files in the `details` directory specified in the source "config.yml" file (see [Source Configure File](Overview.md#source-configure-file)
).

## Building Documentation

### Setup
Currently, the documentation system for MOOSE is under development so the required dependencies are not yet
part of the repository and some setup is required.

1. Create a new python enviornment.

    ```
    conda create --name docs --clone root
    source activate docs
    ```

1. Install the additional python packages.

    ```
    pip install markdown-include python-markdown-math mkdocs-bootstrap mkdocs-bootswatch click jinja2 livereload
    ```

1. Download the MOOSE specific version of [mkdocs]

    ```
    cd ~/projects
    git clone git@github.com:aeslaughter/mkdocs.git
    ```

1. Install the custom version of mkdocs

    ```
    cd mkdocs
    python setup.py install
    ```

### Generate, Serve, and Deploy
To generate, view, and/or build the documentation perform the following.

1. Set your path, so the `moosedocs` executable is available.

    ```
    export PATH=$PATH:~/projects/moose/docs
    ```

1. Move to your application docs directory (where the [Application Configure File](Overview.md#application-configure-file) is located).

    ```
    cd ~/projects/your_app_name_here/docs
    ```

1. Generate the application specific Markdown.

    ```
    moosedocs generate
    ```

1. Serve the page to a local server.

    ```
    moosedocs serve
    ```

1. Deploy the page (location is given by "site_dir" in your [Application Configure File](Overview.md#application-configure-file)).

    ```
    moosedocs build
    ```

[mkdocs](www.mkdocs.org)
