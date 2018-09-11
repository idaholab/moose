# PlaneClipper

The `PlaneClipper` filter allows for regions, defined by a plane and a normal direction, to be
clipped from an existing result.

!listing clipping/clip.py
         start=import
         id=boxclipper-example
         caption=Example script using an `PlaneClipper` object.

!media chigger/clip.png
       style=width:30%;margin:auto;
       id=boxclipper-example-output
       caption=Output from example script using an `PlaneClipper` object.

!chigger options object=chigger.filters.PlaneClipper

!chigger tests object=chigger.filters.PlaneClipper
