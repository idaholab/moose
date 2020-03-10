# EllipseCutUserObject

!syntax description /UserObjects/EllipseCutUserObject

## Overview

The `EllipseCutUserObject` defines the boundary of an elliptical cut for XFEM
to make on a 3 dimensional mesh. The user enters three points in the cut_data
parameter in x, y, z coordinates: the center point of the ellipse (must be
specified first), followed by points on the edge of the ellipse at the
longest and shortest axes (in any order). The object calculates the two radii,
verifies that the points provided are located within the same plane, and has
logic to determine whether a given point is located inside the cut plane.

## Example Input Syntax

!listing test/tests/solid_mechanics_basic/elliptical_crack.i block=UserObjects

!syntax parameters /UserObjects/EllipseCutUserObject

!syntax inputs /UserObjects/EllipseCutUserObject

!syntax children /UserObjects/EllipseCutUserObject
