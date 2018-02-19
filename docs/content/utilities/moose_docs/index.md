# MOOSE Documentation System

As MOOSE and MOOSE-based applications continue to advance, creating quality documentation becomes the
key to gaining new users and providing current users with an experience that allows rapid
development. However, creating quality documentation can be a daunting task, and as a consequence, it
is not uncommon for large projects to have limited, difficult-to-navigate, out-of-date documentation.

To ameliorate this problem, a single-source documentation system was developed based around fully
customizable markdown syntax---[Moose Style Markdown](moose_markdown/index.md)---that includes
features tailored to documenting MOOSE code:

* **Object descriptions and input parameters tables** may be automatically generated from applications.
* **Cross-referencing links to examples and source code** may be created.
* **Dynamic source code listing** allows source code to displayed without duplication and update automatically
with code changes.

The single-source markdown files may be converted into various media forms, including web-sites (like
this one), slide shows, and pdfs (via latex). Most importantly, custom markdown syntax is easily
created so it is possible to develop syntax to meet the needs of any project.

The following links provide additional details on the MOOSE documentation system:

* [Documenting MOOSE](moose_docs/generate.md)
* [Moose Style Markdown](moose_docs/moose_markdown/index.md)
* [Standards for Documentation Pages](moose_docs/docs_standards.md)

If you are creating a website for your own application the following pages may be of use:

* [Setup and Configuration](moose_docs/setup.md)
* [Creating a Web-Site](moose_docs/website.md)
* [Building a Presentation](moose_docs/presentation.md)
* [Generating a PDF](moose_docs/pdf.md)
