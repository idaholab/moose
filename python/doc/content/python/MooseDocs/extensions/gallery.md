# Gallery Extension

The gallery extension provides a mechanism for creating "cards" using the `!card` command and
allows for these items to be organized into a gallery with the `!gallery` command. The
available configuration items for the extension are listed below, in [gallery-config].

!devel settings module=MooseDocs.extensions.gallery
                object=GalleryExtension
                id=gallery-config
                caption=Configuration items for the alert extension.

## Cards

In general, a gallery is composed of cards; however, the 'card' command works as a stand
alone command. The name "card" is derived from the [materialize](https://materializecss.com/cards.html)
framework, which MOOSEDocs relies on for creating website content. The settings for the
card command are listed in [card-settings].

!devel! example id=gallery-example-card
               caption=Example use of the 'card' command.
!card level_set/vortex_out.mp4 title=Vortex Benchmark style=width:50%;
The level set equation is commonly used to for interface tracking, especially when the interface
velocity is known.
!devel-end!

!devel settings module=MooseDocs.extensions.gallery
                object=CardComponent
                id=card-settings
                caption=Settings for the 'card' command within the gallery extension.



## Gallery

A gallery is simply a collection of cards, to create a gallery simply wrap the card commands
with a block-level gallery command as shown below. The available settings for the gallery command
are listed in [gallery-settings].

!devel! example id=gallery-example-gallery
               caption=Example use of the 'gallery' command.
!gallery!
!card level_set/example_circle_64.mp4 title=Translation

!card level_set/circle_rotate_master_out.mp4 title=Rotation

!card level_set/vortex_out.mp4 title=Vortex
!gallery-end!
!devel-end!

!devel settings module=MooseDocs.extensions.gallery
                object=GalleryComponent
                id=gallery-settings
                caption=Settings for the 'gallery' command within the gallery extension.



## Slideshow

A slideshow is a revolving gallery that displays one image at a time and cycles through them,
either automatically on a timer or manually using the on-page controls. This is achieved using
a block-level `!slideshow` command with a `!slide` command inside the block for each slide,
as shown in [gallery-example-slideshow]. Each
slide takes an image and an optional `caption`. The paired form of the command
(`!slide! ... !slide-end!`) also accepts block content, such as a link to the relevant model,
which follows the caption.

The caption and block content appear beneath the image by default, but `overlay_default=True`
may be set on the `!slideshow` command to change the default behavior to have the content
overlaid on the image. Additionally, the choice to overlay the content may be customized
on each slide by setting the `overlay` option on the corresponding `!slide` command.

The slideshow displays an indicator dot for each image and, by default, advances only when the
viewer clicks one. Set the `interval` setting to a number of seconds to advance
automatically; clicking an indicator then stops the automatic advance. The settings for the two commands are listed in
[slideshow-settings] and [slide-settings].

!devel! example id=gallery-example-slideshow
               caption=Example use of the 'slideshow' command.
!slideshow! interval=3 style=max-width:450px;margin-left:auto;margin-right:auto;
!slide application_logos/griffin_description.png caption=Griffin

!slide application_logos/grizzly_description.png caption=Grizzly

!slide application_logos/sockeye_description.png caption=Sockeye
!slideshow-end!
!devel-end!

!devel settings module=MooseDocs.extensions.gallery
                object=SlideshowComponent
                id=slideshow-settings
                caption=Settings for the 'slideshow' command within the gallery extension.

!devel settings module=MooseDocs.extensions.gallery
                object=SlideComponent
                id=slide-settings
                caption=Settings for the 'slide' command within the gallery extension.
