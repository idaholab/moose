# Gallery Extension

The gallery extension provides a mechanism for creating "cards" using the 'card' command and
allow for these items to be organized in to a gallery with the 'gallery' command. The
available configuration items for the extension are listed below, in [gallery-config].

!devel settings module=MooseDocs.extensions.gallery
                object=GalleryExtension
                id=gallery-config
                caption=Configuration items for the alert extension.

## Cards

In general, a gallery is composed of cards; however, the 'card' command works as a stand
alone command. The name card is derived from the [materialize](https://materializecss.com/cards.html)
framework, which MOOSEDocs relies for creating website content. The settings for the
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
