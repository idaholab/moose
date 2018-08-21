# BoxClipper

The `BoxClipper` filter allows for regions, defined by a bounding box, to be clipped from an
existing result.

!listing clipping/box_clip.py
         start=import
         id=boxclipper-example
         caption=Example script using an `BoxClipper` object.

!media chigger/box_clip.png
       style=width:30%;margin:auto;
       id=boxclipper-example-output
       caption=Output from example script using an `BoxClipper` object.

!chigger options object=chigger.filters.BoxClipper

!chigger tests object=chigger.filters.BoxClipper
