# AutoLink Extension

The AutoLink extension provide cross page automatic linking for markdown (*.md) files. This allows
pages, such as those on this site, to link to each other by partial name. In the case of shortcut
style linkes (see [/core.md#shortcut-link]) heading content will also be include automatically.

The following table lists the available configuration options for the AutoLink extension.

!devel settings id=autolink-config caption=Configuration options for the AutoLink extension.
                module=MooseDocs.extensions.autolink object=AutoLinkExtension

## Automatic Links

The link within traditional markdown links (e.g., `[text](link)`) may include a markdown filename.
The name must be unique, but may be incomplete. The entire page hieerachy is searched and filenames
are compared using the python "endswith" method. Thus, the supplied name is considered unique if only
one path from all possible paths ends with the supplied text, see [link-example].

!devel example id=link-example caption=Example automatic link to another markdown page.
[Core](core.md)

The markdown filename link also supports html bookmark style links, as shown in [link-bookmark-example].

!devel example id=link-bookmark-example
               caption=Example of automatic link to another markdown page that includes an html
                       bookmark.
[Core](core.md#shortcut-link)

## Automatic Link Shortcuts

Markdown syntax includes syntax for creating shortcuts (see [core.md#shortcut-link]), within MooseDocs
these are deemed "shortcut links." The AutoLink extension allows for markdown filenames to be used
within a shortcut link. In this case the text from the first heading is used as the link text, as
in [example-shortcut-link].

!devel example id=example-shortcut-link
               caption=Example of a shortcut link that contains a markdown filename.
[core.md]

It is also possible to include html style bookmarks with the filename, depending on the configuration
(see [autolink-config]) the link text will include the name of the text within the bookmark and
optionally the page heading, see [example-shortcut-bookmark-link].

!devel example id=example-shortcut-bookmark-link
               caption=Example of a shortcut linke that contains a markdown file including an html
                       style bookmark.
[core.md#shortcut-link]

## Automatic Source Content

If a filename is used within a link or shortcut link and the file is contained in the git repository
a bottom extending modal window will be created that displays the complete text, as shown in
[example-file-link].

!devel! example id=example-file-link
                caption=Example showing source code links to modal windows with complete source code.
[/Diffusion.C]

[Diffusion Kernel](/Diffusion.C)
!devel-end!
