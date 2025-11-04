# Include Extension

A primary driver behind developing the MooseDocs system was to create a single source
for all documentation and allow the same piece of text to be reused in multiple locations and in
multiple formats. Therefore, it is possible to include markdown within markdown using the
`!include` command.  The available
settings for the `!include` command is given in [include-settings].

!devel settings id=include-settings
                caption=Settings for the `!include` command.
                module=MooseDocs.extensions.include object=IncludeCommand

[example-include] uses the include command to import the complete content of another
file and [example-include-re] imports a portion of a file using the "re" setting.

!devel! example id=example-include
                caption=Example use of the `!include` command to import a complete file.
!include /include_example.md
!devel-end!

!devel! example id=example-include-re
                caption=Example use of the `!include` command to import a portion of another file.
!include /core.md re=(?P<content>^The core.*?)^Syntax
!devel-end!

!alert warning title=Including text with template arguments
See the warning in [extensions/template.md#using-templates] for a caution about
including text that includes template key/value pairs.

[markdown]: https://en.wikipedia.org/wiki/Markdown
