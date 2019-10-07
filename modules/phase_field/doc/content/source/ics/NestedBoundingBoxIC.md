# NestedBoundingBoxIC

NestedBoundingBoxIC allows setting the initial condition of a value of a field inside multiple nested
bounding boxes and the value outside the outermost box. Each box is axis-aligned and is specified by
passing in the x,y,z coordinates of the corners with the smallest and the largest coordinates for each
box. The order of defining the box coornidates must be from the innermost to the outermost box. An
interfacial width can be assigned for diffused interfaces. Partially overlapping boxes are not supported.

## Example input:

!listing NestedBoundingBoxIC_2D.i block=ICs

!media media/phase_field/nested_bounding_box.png
       caption=Initial condition of the variable defined in the above example.
       style=width:80%;display:block;margin-left:auto;margin-right:auto;

## Class Description

!syntax description /ICs/NestedBoundingBoxIC

!syntax parameters /ICs/NestedBoundingBoxIC

!syntax inputs /ICs/NestedBoundingBoxIC

!syntax children /ICs/NestedBoundingBoxIC

!bibtex bibliography
