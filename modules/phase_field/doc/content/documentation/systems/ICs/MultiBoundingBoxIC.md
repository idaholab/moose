# MultiBoundingBoxIC

MultiBoundingBoxIC allows setting the initial condition of a value of a field inside and outside
multiple bounding boxes. Each box is axis-aligned and is specified by passing in the x,y,z
coordinates of opposite corners for each box.

## Example input:

In the example input that you may either supply a single "inside" value, or one value for
each box.

!listing MultiBoundingBoxIC2D.i block=ICs

## Class Description

!syntax description /ICs/MultiBoundingBoxIC

!syntax parameters /ICs/MultiBoundingBoxIC

!syntax inputs /ICs/MultiBoundingBoxIC

!syntax children /ICs/MultiBoundingBoxIC

!bibtex bibliography
