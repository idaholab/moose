# LineSegmentCutSetUserObject

!syntax description /UserObjects/LineSegmentCutSetUserObject

## Overview

The `LineSegmentCutSetUserObject` defines a set of line segment cuts for XFEM
to make on a 2 dimensional mesh. The cut_data parameter consists of $n$ entries
each with a length of six Real values (the $n$ entries are not separate but
rather the length of the cut_data vector must be a multiple of six). Each set
of six values in cut_data prescribes a line segment cut: start point x, start
point y, end point x, end point y, start cut time, end cut time. The object
checks to ensure that the length of cut_data is a multiple of six. Provided
that the start cut time and end cut time values are different, the cut will
lenghten by $r_{total} \cdot \frac{t_{current} - t_{start}}{t_{end} -
t_{start}}$ at each timestep from start cut time until the specified end cut
time. As in the `LineSegmentCutUserObject`, scaling and translation are
available via optional parameters and apply to all applicable components of
cut_data.

## Example Input Syntax

!listing test/tests/second_order_elements/square_branch_quad9_2d.i block=UserObjects

!syntax parameters /UserObjects/LineSegmentCutSetUserObject

!syntax inputs /UserObjects/LineSegmentCutSetUserObject

!syntax children /UserObjects/LineSegmentCutSetUserObject
