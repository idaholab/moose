# Devel Extension

## Examples

!devel example
This is a +test+ without float.


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


## Eq. in Example

!devel! example caption=Test with Equations
[foo] is famous.

!equation id=foo
E = mc^2
!devel-end!
