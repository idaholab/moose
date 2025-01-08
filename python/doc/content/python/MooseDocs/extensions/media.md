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

The media extension supports including the standard HTML image extensions: png, gif, jpg, jpeg,
and svg. Images are added using the `!media` command followed by the filename, as shown in
[example-media]. [image-settings] includes the list of available settings for the media command for
images.

!alert note
The "style" setting may be used, as in the example below, to control the placement and size of the
image.

!devel! example id=example-media caption=Example of the media command with an image.
!media large_media/framework/inl_blue.png style=width:25%;float:right;margin-left:30px;

INL is part of the U.S. Department of Energy's complex of national laboratories. The laboratory
performs work in each of the strategic goal areas of DOE: energy, national security, science and
environment. INL is the nation's leading center for nuclear energy research and development. INL is
the nation's lead laboratory for nuclear energy research, development, demonstration and deployment
and we are engaged in the mission of ensuring the nation's energy security with safe, competitive and
sustainable energy systems and unique national and homeland security capabilities.
!devel-end!

!devel settings module=MooseDocs.extensions.media
                object=ImageCommand
                id=image-settings
                caption=Settings available for images when using the media command.

## Images Generated On-the-fly with Plot Scripts id=using-plot-script

There are some applications where an image may change very frequently, such as
an image generated for regular assessment runs. In this case, it is undesirable
to store a new image file in the application repository with each code version.
Thus the media extension provides the ability to generate images on-the-fly by
providing a python plot script name.

[example-plot-script] gives an example of the python script capability, and
[plot-script-listing] displays the plot script used in the example.

!devel! example id=example-plot-script caption=Example of how to generate a plot on-the-fly from a python script.
!media example_plot.py
       id=example-plot
       caption=Example plot.
       style=width:50%;padding:20px;
!devel-end!

!listing example_plot.py id=plot-script-listing caption=Example plot script.

!alert! note title=Specifying the image name.
The image name may be specified with the key `image_name`, e.g.,

```
!media example_plot.py
       image_name=some_plot.jpg
       id=example-plot
       caption=Example plot.
       style=width:50%;padding:20px;
```

or if not provided, the image is assumed to have the same base name as the plot
script, but with the `.png` extension instead of `.py`. In [example-plot-script],
the plot script was named `example_plot.py`, and the `image_name` key was omitted,
so the image name `example_plot.png` was assumed.
!alert-end!

!alert! warning title=Always change to the plot script's directory.
Note the line

```
os.chdir(os.path.dirname(os.path.realpath(__file__)))
```

in the example plot script, which changes the directory to the plot script's
directory. Otherwise, the plot script is not necessarily run from the directory
containing it, in which case the relative path to the data (e.g., CSV) file would
be incorrect, leading to a file-not-found error.
!alert-end!

[script-settings] lists the available settings for the media plot script command.

!devel settings module=MooseDocs.extensions.media
                object=ScriptCommand
                id=script-settings
                caption=Settings available for plot scripts when using the media command.

## Videos

Locally stored or hosted videos can be displayed using the `!media` syntax. This works in the same
fashion as for [images](#images), but also includes some extra settings as listed in
[video-settings].

!media https://upload.wikimedia.org/wikipedia/commons/transcoded/c/c0/Big_Buck_Bunny_4K.webm/Big_Buck_Bunny_4K.webm.1080p.vp9.webm
       id=big_buck_bunny
       caption=["Big Buck Bunny"](https://en.wikipedia.org/wiki/Big_Buck_Bunny) is an open-source
               animated short.

YouTube videos can also be displayed. The embedded youtube URL is simply provided as the media, this
URL must contain 'www.youtube.com' and not end with an extension. Similar to locally stored or
hosted videos, extra settings are provided as listed in [video-settings].

!media https://www.youtube.com/embed/2tJwBsYaLaI
       id=training-webinar
       caption=MOOSE training webinar given on June 9--10, 2020.

!devel settings module=MooseDocs.extensions.media
                object=VideoCommand
                id=video-settings
                caption=Settings available for videos when using the media command.

## Float Images/Videos

As is the case for many items within the MooseDocs system (i.e., [extensions/table.md]), it is
possible to create numbered images that may be referenced, as shown in [example-image-float].
The "id" and "caption" settings are available for both images and videos.

!devel! example id=example-image-float caption=Example image with caption and numbered prefix.
!media large_media/framework/inl_blue.png
       id=inl-logo
       caption=The Idaho National Laboratory logo.
       style=width:50%;padding:20px;
!devel-end!

### Float Referencing

Referencing floats is possible using the `!ref` inline command, from both the same page and other pages:

!devel! example id=example-ref-float caption=Example of referencing floats.
Float from the same page: [!ref](inl-logo)

Figure from different page: [!ref](graph.md#plotly-ext-config)

Table from different page: [!ref](table.md#table-floating)

Algorithm from different page: [!ref](algorithm.md#bk)
!devel-end!

!devel settings module=MooseDocs.extensions.floats
                object=FloatReferenceCommand
