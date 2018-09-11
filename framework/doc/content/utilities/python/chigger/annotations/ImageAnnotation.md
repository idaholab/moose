# ImageAnnotation

The `ImageAnnotaion` object is designed to add 2D image objects within the render window. The image
must be a [PNG](https://en.wikipedia.org/wiki/Portable_Network_Graphics). As with most chigger
objects the size is given as relative to the viewport via the position and the width of the image,
the aspect ratio of the image is maintained, see [image-annotation] and the resulting
image in [image-annotation-output].

!listing image_annotation.py
         id=image-annotation
         start=import
         caption=Example of creating a `ImageAnnotation` placing images with the render window.

!media image_annotation.png
       style=width:30%;margin:auto;
       id=image-annotation-output
       caption=Resulting image from running the `image_annotation.py` script.

!chigger options object=chigger.annotations.ImageAnnotation

!chigger keybindings object=chigger.annotations.ImageAnnotation

!chigger tests object=chigger.annotations.ImageAnnotation
