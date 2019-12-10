# IsolatedBoundingBoxIC

IsolatedBoundingBoxIC allows setting the initial condition of a value of a field inside and outside
multiple isolated bounding boxes. Each box is axis-aligned and is specified by passing in the x,y,z
coordinates of the corners with the smallest and the largest coordinates for each box. An interfacial
width can be assigned for diffused interfaces.

## Example input:

!listing IsolatedBoundingBoxIC_2D.i block=ICs

!media media/phase_field/isolated_bounding_box.png
       caption=Initial condition of the variable defined in the above example.
       style=width:80%;display:block;margin-left:auto;margin-right:auto;

## Class Description

!syntax description /ICs/IsolatedBoundingBoxIC

!syntax parameters /ICs/IsolatedBoundingBoxIC

!syntax inputs /ICs/IsolatedBoundingBoxIC

!syntax children /ICs/IsolatedBoundingBoxIC

!bibtex bibliography
