# RectangleCutUserObject

!syntax description /UserObjects/RectangleCutUserObject

## Overview

The `RectangleCutUserObject` defines the boundary of a rectangular cut for XFEM
to make on a 3 dimensional mesh. The cut_data parameter is a vector of length
twelve that defines the x, y , and z Real values of the four vertices that form
the rectangular cut. The object calculates the midpoint of the rectangular cut
and normal, and includes logic to determine if a given point is located within
the cut plane.

## Example Input Syntax

!listing test/tests/solid_mechanics_basic/edge_crack_3d.i block=UserObjects

!syntax parameters /UserObjects/RectangleCutUserObject

!syntax inputs /UserObjects/RectangleCutUserObject

!syntax children /UserObjects/RectangleCutUserObject
