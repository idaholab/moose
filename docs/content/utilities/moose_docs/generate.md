# Generating New Documentation for MooseObjects

Clear and consistent documentation is a necessary component of code developement within MOOSE.
It is expected that developers submitting new classes to the MOOSE and MOOSE module repository will create a corresponding markdown file, using
[MOOSE Flavored Markdown](moose_markdown/index.md), to document the new classes.
The created file must be stored in a file named according to the registered MOOSE syntax within the "install" directory explained in the [Configuration](moose_docs/setup.md#configuration) section.

Developers who modify existing classes should update the corresponding markdown documentation file.

## Generating Templates for New MooseObjects
If you are writing a documentation for a new class with MOOSE or the modules, you can use the MooseDocs executable to build a documentation stub for your new class; the stub includes a markdown documentation template.
The markdown documentation template stub adheres to the documentation page standards described in the [Standards for Documentation Pages](moose_docs/docs_standards.md) and includes examples of common MooseDocs extensions, including equations and references.

Template stubs for the systems and objects in MOOSE-based applications can be generated using the "moosedocs.py" script, using the command as follows:

```text
cd ~/projects/moose/docs
./moosedocs.py check --generate
```

To see a list of additional options for this command run with the "-h" flag.
```text
./moosedocs check -h
```

Ensure that your source code follows the [Source Code Documentation Standards](moose_docs/code.md) for the MooseDocs automatic linking, provided by the `!syntax` commands, to function properly.

This generate command needs to be run only when you edit source code of your application:

  * When you add a new object (e.g., Kernel, BoundaryCondition, etc.) to your application,
  * When you add or modify the class description of an existing object, or
  * When you modify the input parameters of an existing object.

!!! warning "Generating Overwrites 'pages.yml'"
    When the generate command is used the 'pages.yml' files will be updated to match the current structure. It is
    possible to disable this behavior, see the help ('./moosedocs check -h') for more information.


## Standards for Documentation Pages
Once created these documentation stub files should be modified with additional details, following the [Standards for Documentation Pages](moose_docs/docs_standards.md) guidelines.
The completed markdown documentation pages are then available for use when generating a [web-site](utilities/moose_docs/website.md), [presentations](utilities/moose_docs/presentation.md), or [PDFs](utilities/moose_docs/pdf.md).
