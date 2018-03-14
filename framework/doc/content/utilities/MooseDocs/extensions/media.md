# Media Extension

The media extension provides the `!media` markdown command for adding images and videos. As used
throughout MooseDocs content, the `!media` command can optionally create a numbered
[extensions/floats.md] by specifying the "id" setting. A caption may be include by using the
"caption" setting.

The configuration options for the media extension are listed in [config-media-ext].

!devel settings id=config-media-ext
                caption=Configuration options for the media extension.
                module=MooseDocs.extensions.media
                object=MediaExtension

## Images

The media extension supports including the standard html image extensions: png, gif, jpg, jpeg,
and svg. Images are added using the !media command followed by the filename, as shown in
[example-image].

!devel! example id=example-image caption=Example image with caption and numbered prefix.
!media media/inl_blue.png
       id=inl-logo
       caption=The Idaho National Laboratory logo.
!devel-end!

!devel settings module=MooseDocs.extensions.media object=ImageCommand


## Videos

Locally stored or hosted videos can be displayed using the `!media` syntax. This works in the same
fashion as for [images](#images), but also includes some extra settings as listed in the section
below.

!media http://clips.vorwaerts-gmbh.de/VfE.webm
       id=big_buck_bunny
       caption=["Big Buck Bunny"](https://en.wikipedia.org/wiki/Big_Buck_Bunny) is an open-source animated short.

!devel settings module=MooseDocs.extensions.media object=VideoCommand
