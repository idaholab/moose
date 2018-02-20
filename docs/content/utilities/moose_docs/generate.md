# Documenting MOOSE

In order to ensure that MOOSE documentation continues to improve requirements exist for
docuementing new and changing code within MOOSE:

1. Any new MooseObject must have an associated markdown description page.
2. Developers who modify existing classes should update the existing markdown documentation file.
3. All new tests must contain a 'requirement' and link to 'design' and 'issues'.

## New MooseObjects

Clear and consistent documentation is a necessary component of code developement within MOOSE.  It is
expected that developers submitting new MooseObjects (e.g., `Kernel` or `BoundaryCondition`) to the
MOOSE and MOOSE module repository will create a corresponding markdown file, using [MOOSE Flavored
Markdown](moose_markdown/index.md), to document the new classes.  The created file must be stored in
a file named according to the registered MOOSE syntax within the "install" directory explained (see
[Configuration](moose_docs/setup.md#configuration) section for details).

If you are writing a documentation for a new class with MOOSE or the modules, you can use the
MooseDocs executable to build a documentation stub for your new class; the stub includes a markdown
documentation template.  The markdown documentation template stub adheres to the documentation page
standards described in the [Standards for Documentation Pages](moose_docs/docs_standards.md) and
includes examples of common MooseDocs extensions, including equations and references.

Template stubs for the systems and objects in MOOSE-based applications can be generated using the
"moosedocs.py" script, using the command as follows:

```bash
cd ~/projects/moose/docs
./moosedocs.py check --generate
```

Ensure that your source  follows the [Source Code Documentation Standards](moose_docs/code.md)
for the MooseDocs automatic linking, provided by the `!syntax` commands, to function properly.

This generate command needs to be run only when you edit source code of your application:

  * When you add a new object (e.g., Kernel, BoundaryCondition, etc.) to your application, When you
  * add or modify the class description of an existing object, or When you modify the input
  * parameters of an existing object with the `validParams` function.

After the stub files have been created the a local, live version of the website should
be used to add content (see the [Live Website](#live-website) section below).

## Live Website

When adding documentation it is useful to create a create a live version of the MOOSE website. To
create the site MOOSE modules executable must exist, this is accomplished by using the following
commands.

```bash
cd ~/projects/moose/modules
make -j16 # 16 should be replaced by the number of cores on your system
```

Next, the website content must be built and served, this is done by running the following commands.
When this command completes, which takes several minutes, a message will be printed and the site will
be hosted at [http://127.0.0.1:8000](http://127.0.0.1:8000).

```bash
cd ~/projects/moose/docs
./moosedocs build --serve
```

Once the server is running the markdown files within the repository may be modified, when the
changes are saved the local server will automatically update the content.
The content added or modified should follow the [Standards for Documentation
Pages](moose_docs/docs_standards.md) guidelines.

## Requirement, Design, and Issues

MOOSE follows strict software quality standards, to meet these standards every test within MOOSE
must provide three items with each test of the test specifcation.

1. **requirement**: A sort description that explains what the "requirement" that the test is
testing. The text for the requirement must be listed in the test specification file ("tests" file).

1. **design**: A list of markdown files associated with the test that explain the systems, objects,
and design documents. The file paths do not need to be complete, but must be unique.

1. **issues**: A list of [github issues](https://github.com/idaholab/moose/issues/) that are
associated with the test and items being tested.

The complete list of all the requirements and associated test for MOOSE is provided in the
[Software Requirement Specification](/moose_srs.md).
