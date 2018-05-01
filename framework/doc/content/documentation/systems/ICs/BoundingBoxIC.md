# BoundingBoxIC

BoundingBoxIC allows setting the initial condition of a value inside and outside of a specified
box. The box is aligned with the x, y, z axes and is specified by passing in the x, y, z
coordinates of the bottom left point and the top right point. Each of the coordinates of the
"bottom_left" point MUST be less than those coordinates in the "top_right" point.

When setting the initial condition, if `bottom_left <= Point <= top_right` then the "inside" value is used.
Otherwise the "outside" value is used.

!alert note
When using this IC, only a single bounding box my be specified within the domain. If multiple bounding
boxes are needed, this capability is implemented in the phase_field module as `MultiBoundingBoxIC`.

## Class Description

!syntax description /ICs/BoundingBoxIC

!syntax parameters /ICs/BoundingBoxIC

!syntax inputs /ICs/BoundingBoxIC

!syntax children /ICs/BoundingBoxIC

!bibtex bibliography
