## Setup and Configuration
Before discussing the required configuration file it is import to understand how the MooseDocs
system operates. The system relies heavily on the [python-markdown] package, which is a tool written
in [python] to convert from [markdown] syntax to [html]. For websites the MooseDocs system simply
converts all [markdown] files (those with a *.md extension) automatically, in parallel, using
[python-markdown]. For [presentation](moose_docs/presentation.md) and [pdf](moose_docs/pdf.md) creation, the
conversion only acts on a specified, single markdown file.

For all conversion types a list of [python-markdown extensions](https://pythonhosted.org/Markdown/extensions/index.html) must be provided. A detailed
description of the extensions is provided on the [MOOSE markdown](moose_docs/moose_markdown/index.md) pages. This is done via a
configuration file, which is nothing more than a list of extensions and extension configuration
options needed to convert from markdown to the desired format. The configuration file is written
using [YAML] format.s
