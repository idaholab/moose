# LineSegmentCutUserObject

!syntax description /UserObjects/LineSegmentCutUserObject

## Overview

The `LineSegmentCutUserObject` defines a line segment cut for XFEM to make on a
2 dimensional mesh. The start and end points of the line segment are defined in
cut_data as a vector of four Real values: start point x, start point y, end
point x, end point y. By default, the entire cut specified is performed at
$t=0$. Propogation of the cut may be prescribed along the line
segment by specifying the time_start_cut and time_end_cut parameters in
simulation time. The cut will lengthen by $r_{total} \cdot \frac{t_{current} -
t_{start}}{t_{end} - t_{start}}$ at each timestep from time_start_cut until
time_end_cut. There are also options to scale and translate the points given in
cut data. The scale and translation of the x and y coordinates must be
specified separately, and if specified, the scale and translation are applied
to both start and end points.

## Example Input Syntax

!listing test/tests/second_order_elements/diffusion_2d_quad8.i block=UserObjects

!syntax parameters /UserObjects/LineSegmentCutUserObject

!syntax inputs /UserObjects/LineSegmentCutUserObject

!syntax children /UserObjects/LineSegmentCutUserObject
