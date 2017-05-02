# IncludeExtension

The main driver behind developing the MooseDocs documentation system was to create a single source
for all documentation and allow the same piece of text to be reused in multiple locations and in
multiple formats. Therefore, it is possible to include markdown within markdown using the
`!include` command. For example, \ref{include-example} provides the include command that is used
below to capture text from the [global](extensions/global.md) extension.

!listing id=include-example caption=An example `!include` command for retrieving markdown from another file.
```
!include docs/content/utilities/moose_docs/moose_markdown/extensions/global.md re=The\sglobal.*?applied\.
```

The following content is extracted from the [global](extensions/global.md) extension:
!include docs/content/utilities/moose_docs/moose_markdown/extensions/global.md re=The\sglobal.*?applied\.

!extension IncludeExtension caption=Configure Options for IncludeExtension

!extension-settings moose-markdown-include title=Settings for `!include` Command
