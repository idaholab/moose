MOOSE includes tools for auto-generating web-sites (like this one) that documents your application. Included
in this tool is the ability to:

* Add detailed descriptions of custom systems and objects using [Moose Style Markdown](moose_flavored_markdown.md),
which has useful features designed for documenting source code and input file syntax;
* Generate pages based on class descriptions and input parameter documentation included in your source code;
* Generate links to examples and source code that use or inherit from your objects;
* Create summary tables of objects included in your application; and
* Include documentation from the MOOSE framework, modules, or other applications that are registered in your application.

There are two primary locations that documentation should be added: Source Code and Markdown Descriptions.

---

## Source Code Documentation
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

## Object Description Markdown
A detailed description may be provided to the generated documentation for an object by creating a markdown file using
[MOOSE Flavored Markdown](moose_flavored_markdown.md). The created file must be stored in a file named according to the
registered MOOSE syntax within the "details" directory explained in the [Configuration](#configuration) section below.
For example, the details for the [Diffision](/Kernels/Diffusion.md) are in the `framework/docs/details/Kernels/Diffusion.md`
file.

---

## Site Pages

The final step when preparing an application site is to define the website page layout, this is done in "pages.yml" within
the docs directory. The information within this file mimics the [mkdocs pages](http://www.mkdocs.org/user-guide/configuration/#pages)
configuration, with one exception: it is possible to include other markdown files. For example, the "pages.yml" for
the MOOSE website includes the following.

!text docs/pages.yml

Notice, that the framework and the modules each have include statements pointing to another "pages.moose.yml" files. This
file is generated for object in the application and is placed in the "install" directory (see [Setup](/content/utilities/documentation/setup.md)).

---

## Generate Documentation
To generate the documentation from your application, first your application must be compiled. Then following
should be executed:

Move to your application docs directory (where the [Configure File](#configuration) is located).
```text
cd ~/projects/your_app_name_here/docs
```

Generate the application specific Markdown.

```text
./moosedocs.py generate
```

To see a list of additional options for this command run with the "-h" flag.
```text
./moosedocs generate -h
```

This command needs to be run only when you edit source code of your application:

* when you add a new object (e.g., Kernel, BoundaryCondition, etc.) to your application,
* when you add or modify the class description of an existing object, and
* when you modify the input parameters of an existing object.

---

## View your Documentation
When you want to view your documentation website, you must serve the site to a local server. This is done as:
```text
./moosedocs.py serve
```

To see a list of additional options for this command run with the "-h" flag.
```text
./moosedocs serve -h
```

---

## Deploy your Website
To deploy the website, run the script with the 'build' option. This will create the site in the location is given
by "site_dir" in your MkDocs Configure File (mkdocs.yml). This site can then be copied to a server for
hosting.

```text
./moosedocs.py build
```

To see a list of additional options for this command run with the "-h" flag.
```text
./moosedocs build -h
```
