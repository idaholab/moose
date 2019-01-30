# Devel Extension

## Examples

!devel example id=markdown-example caption=The example command.
This is a +test+.


!devel! example id=markdown-example2 caption=The example command with block format.
First paragraph.

Second paragraph.
!devel-end!


## Settings

The available settings for code blocks are listed in [code-settings].

!devel settings module=MooseDocs.extensions.core object=CodeBlock

!devel settings module=MooseDocs.extensions.core object=CodeBlock
       caption=Settings for code blocks.
       id=code-settings
