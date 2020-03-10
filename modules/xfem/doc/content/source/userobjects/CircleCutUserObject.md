# CircleCutUserObject

!syntax description /UserObjects/CircleCutUserObject

## Overview

The `CircleCutUserObject` defines the boundary of a circular cut for XFEM to
make on a 3 dimensional mesh. The user enters three points in the cut_data
parameter in x, y, and z coordinates: first the center point of the circle,
followed by two points on the boundary of the circle. The object calculates
the radius and angle of the circle (with respect to the x-y plane), verifies
that the points define a circle (rather than an ellipse), and has logic to
determine if a given point is inside the cut plane.

## Example Input Syntax

!listing test/tests/solid_mechanics_basic/penny_crack.i block=UserObjects

!syntax parameters /UserObjects/CircleCutUserObject

!syntax inputs /UserObjects/CircleCutUserObject

!syntax children /UserObjects/CircleCutUserObject
