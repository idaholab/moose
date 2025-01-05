# MOOSE Documentation System (MooseDocs)

As MOOSE and MOOSE-based applications continue to advance, creating quality documentation becomes the
key to gaining new users and providing current users with an experience that allows rapid
development. However, creating quality documentation can be a daunting task, and as a consequence, it
is not uncommon for large projects to have limited, difficult-to-navigate, out-of-date documentation.

To ameliorate this problem, a single-source documentation system was developed based around fully
customizable markdown syntax---[Moose Style Markdown](MooseDocs/specification.md)---that includes
features tailored to documenting MOOSE code:

- C++ [`MooseObject`](MooseObject.md) descriptions and input parameters tables may be automatically generated from
  applications (see [/appsyntax.md]).
- Cross-referencing links to examples and source code may be created (see [/autolink.md]).
- Dynamic source code listings allow source to be displayed without duplication and update
  automatically with code changes (see [/listing.md]).

The single-source markdown files may be converted into various media forms, including web-sites (like
this one), slide shows, and pdfs (via LaTeX). Most importantly, custom markdown syntax is easily
created so it is possible to develop syntax to meet the needs of any project.

The following links provide additional details on the MOOSE documentation system:

- [MooseDocs/specification.md]
- [framework/documenting.md]
- [MooseDocs/standards.md]

If you are creating a website for your own application the following page may be of use:

- [MooseDocs/setup.md]
- [MooseDocs/config.md]
