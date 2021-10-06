# AutoLink Extension

The AutoLink extension provides cross-page automatic linking for markdown (*.md) files. This allows
pages, such as those on this site, to link to each other by partial name. In the case of shortcut
style links (see [/core.md#shortcut-link]), heading content may also be included automatically. [autolink-config] lists the available configuration options for the AutoLink extension.

!devel settings id=autolink-config caption=Configuration options for the AutoLink extension.
                module=MooseDocs.extensions.autolink object=AutoLinkExtension

## Automatic Links id=autolinks

In MooseDown, the link within traditional markdown link syntax (e.g., `[text](link)`) may be a markdown filename.
The name must be unique, but may be incomplete. The entire page hierarchy is searched and filenames
are compared using the python `str.endswith()` method. Thus, the supplied name is considered unique if only
one path from all possible paths ends with the supplied text, see [link-example].

!devel example id=link-example caption=Example automatic link to another markdown page.
[Core](core.md)

The syntax also supports HTML bookmark style links, as demonstrated in [link-bookmark-example].

!devel example id=link-bookmark-example
               caption=Example of automatic link to another markdown page that includes an HTML
                       bookmark.
[Core](core.md#shortcut-link)

Optional key-value pairs used to define the settings for automatic links can be specified like `[text](link key=value)`. A complete list of the available settings is given in [link-settings]. The `alternative` setting is particularly powerful and serves as a substitute for the originally linked file should it fail to be found. The input for this setting may be an HTML bookmark, a URL, or a markdown file plus bookmark combination (or just a file) as demonstrated in [alternative-link-example].

!devel! example id=alternative-link-example
                caption=Example uses of the `alternative` setting for automatic links when the original markdown file is not found in the available content.
[Bookmark](not_a_real_directory/not_a_real_file_name.md alternative=#source)

[URL](not_a_real_file_name.md alternative=https://www.google.com/)

[File and bookmark](not_a_real_file_name.md#not-a-real-bookmark alternative=core.md#shortcut-link)
!devel-end!

The `optional` and `exact` settings are similarly applied when handling alternative links to markdown files.

!devel settings module=MooseDocs.extensions.autolink
       object=PageLinkComponent
       id=link-settings
       caption=Available settings for automatic links. Note that the `language` setting has no effect unless the link is a [source file](#source).

!alert warning title=Try not to abuse alternative links.
The `alternative` setting should not be used as a means to circumvent potentially broken content configurations, but rather consciously under special circumstances. For example, its main purpose is to provide a fallback when developing sites designed with [multiple configurations](MooseDocs/config.md#multiconfigs) and cross-links between them, but expeditiously building only one of those configurations such that certain content is only conditionally unavailable.

<!--TODO: The above alert should include a link to the appropriate multiconfigs documentation when it becomes available. See #18137-->

## Automatic Link Shortcuts

Markdown includes syntax for creating shortcuts (see [core.md#shortcut-link]). In MooseDown,
these are deemed "shortcut links." The AutoLink extension allows for markdown filenames to be used
within a shortcut link. In this case, the text from the first heading is used as the link text as
in [example-shortcut-link].

!devel example id=example-shortcut-link
               caption=Example of a shortcut link that contains a markdown filename.
[core.md]

It is also possible to include HTML style bookmarks with the filename. Depending on the configuration
(see [autolink-config]), the link text will include the name of the text within the bookmark and
optionally the page heading, see [example-shortcut-bookmark-link].

!devel example id=example-shortcut-bookmark-link
               caption=Example of a shortcut link that contains a markdown file including an HTML
                       style bookmark.
[core.md#shortcut-link]

A complete list of the available settings for automatic link shortcuts is given in [shortcut-link-settings]. Notice that the `alternative` setting is available and its use is demonstrated in [example-shortcut-alternative-link].

!devel! example id=example-shortcut-alternative-link
                caption=Example uses of the `alternative` setting for automatic shortcut links when the original markdown file is not found in the available content.
[not_a_real_file_name.md alternative=#autolinks]

[not_a_real_file_name.md alternative=core.md]
!devel-end!

Note that explicit URLs like `https://www.google.com/` are not supported for automatic shortcut link alternatives, and MooseDocs will log an error if such an attempt is made.

!devel settings module=MooseDocs.extensions.autolink
       object=PageShortcutLinkComponent
       id=shortcut-link-settings
       caption=Available settings for automatic link shortcuts. Note that the `language` setting has no effect unless the link is a [source file](#source).

!alert note title=Applicable settings
The `alternative`, `optional`, and `exact` settings given in both [link-settings] and [shortcut-link-settings] have no effect when the link is a local bookmark, e.g., `[#bookmark]`, nor when it is a [source link](#source). Furthermore, the `language` setting is only applicable to source links.

## Automatic Source Content id=source

If a filename is used within a link or shortcut link and the file is contained in the git repository
a bottom extending modal window will be created that displays the complete text, as shown in
[example-file-link].

!devel! example id=example-file-link
                caption=Example showing source code links to modal windows with complete source code.
[/Diffusion.C]

[Diffusion Kernel](/Diffusion.C)
!devel-end!

Similar to the [python/MooseDocs/extensions/listing.md], the optional `language` setting can be used to specify the coding language to use for syntax highlighting as demonstrated in [example-file-link-language].

!devel! example id=example-file-link-language
                caption=Example showing source code link that sets the language to use for syntax highlighting.
[`run_tests`](/test/run_tests language=python)
!devel-end!
