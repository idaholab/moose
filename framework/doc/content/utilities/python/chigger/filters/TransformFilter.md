# TransformFilter

The `TransformFilter` filter allows for existing geometry to be transformed, such as scaling the
result in the x coordinate direction.

!listing transform/scale.py
         start=import
         id=transform-example
         caption=Example script using an `TransformFilter` object.

!media chigger/scale.png
       style=width:30%;margin:auto;
       id=transform-example-output
       caption=Output from example script using an `TransformFilter` object.

!chigger options object=chigger.filters.TransformFilter

!chigger tests object=chigger.filters.TransformFilter
