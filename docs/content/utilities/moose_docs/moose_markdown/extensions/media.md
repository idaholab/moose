# MediaExtension

!media docs/media/memory_logger-plot_multi.png width=30% padding-left=20px float=right id=memory_logger caption=The [memory_logger](/memory_logger.md) is a utility that allows the user to track the memory use of a simulation.

The media extension provides the `!media` markdown command for adding images, slideshow, and videos.
As used throughout this section, the `!media` command can optionally created numbered "floats" by
specifying the "id", as detailed on the [Numbered Floats](extensions/numbered_floats.md) page.

## Images

It is possible to include images with more flexibility than standard markdown. For example, \ref{memory_logger} was produced using the markdown listed in \ref{image-example}.

The markdown keyword for MOOSE images is `!media` followed by the filename as shown below. This
command, like most of the other special MOOSE markdown commands except arbitrary html attributes
that are then applied to the "style" attribute.

!listing id=image-example caption=Markdown used in this document to produce \ref{memory_logger}.
```markdown
!media docs/media/memory_logger-plot_multi.png width=30% padding-left=20px float=right caption=The [memory_logger](/memory_logger.md) is a utility that allows the user to track the memory use of a simulation.
```

!extension-settings moose_image title=Image Command Settings (`!media`)

## Videos

Locally stored or hosted videos can be displayed using the `!media` syntax. This works in the same
fashion as for [images](#images), but also includes some extra settings as listed in the section
below.

!media http://clips.vorwaerts-gmbh.de/VfE.webm video-width=100% id=big_buck_bunny caption=["Big Buck Bunny"](https://en.wikipedia.org/wiki/Big_Buck_Bunny) is an open-source animated short.

!extension-settings moose_video title=Video Command Settings (`!media`)

## Slideshows
A sequence of images can be shown via a ["slider"](http://materializecss.com/media.html#slider). For example, \ref{slider-example} produces the slider shown in \ref{slider-figure}

!listing id=slider-example caption=Syntax for the `!media` command that created \ref{slider-figure}.
```markdown
!media style=width:50%;margin-left:auto;margin-right:auto; id=slider-figure caption=Example "slider".
    docs/media/memory_logger-darkmode.png caption= Output of memory logging tool position=relative left=150px top=-150px
    docs/media/testImage_tallNarrow.png background-color=#F8F8FF caption= This is a tall, thin image color=red font-size=200% width=200px height=100%
    docs/media/github*.png background-color=gray
    docs/media/memory_logger-plot_multi.png
```

!media style=width:50%;margin-left:auto;margin-right:auto; id=slider-figure caption=Example "slider".
    docs/media/memory_logger-darkmode.png caption= Output of memory logging tool position=relative left=150px top=-150px
    docs/media/testImage_tallNarrow.png background-color=#F8F8FF caption= This is a tall, thin image color=red font-size=200% width=200px height=100%
    docs/media/github*.png background-color=gray
    docs/media/memory_logger-plot_multi.png

!extension-settings moose_slider title=Slider Command Settings (`!media`)

!extension MediaExtension title=MediaExtension Configuration Options
