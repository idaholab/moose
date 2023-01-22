# Style Extension

The style extension exists to apply, as the name suggests, style to markdown text via the "style"
command. The list of available configuration options is provided in [style-extension-config].

## Extension Configuration

!devel settings module=MooseDocs.extensions.style
                object=StyleExtension
                id=style-extension-config
                caption=Available configure options for the StyleExtension object.

## Style Command

Style can be applied to blocks of text via the block versions of the "style" command. For example,
to center text horizontally use the "halign" setting as follows.

!devel! example id=style-block-example caption=Example use of "style" command for a block of text.
!style halign=center
This text should be centered.
!devel-end!

!devel! example id=style-block-example2
                caption=Example use of "style" command for a block of text with multiple paragraphs.
!style! halign=center
This text should be centered.

As should this text.
!style-end!
!devel-end!


Inline text may also be styled, for example the following creates colored text within a box.

!devel! example id=style-inline-example caption=Example use of "style" command for inline text.
It is possible to create [!style color=red border=2](red text within a box) using the "style"
command.
!devel-end!



The complete list of available settings is provide in [style-command-settings].


!devel settings module=MooseDocs.extensions.style
                object=StyleCommand
                id=style-command-settings
                caption=Available settings for the 'style' command.
