# QuadSubChannelNormalSliceValues

!syntax description /Outputs/QuadSubChannelNormalSliceValues

## Overview

<!-- -->

This kernel is used to save the solution variables in a file, for the square subchannel problem.
Imagine a plane perpendicular to the flow direction at a specific height defined by the user.
The solution variables defined by the user are interpolated at the intersection of that plane
and the subchannel mesh and printed on that file as a matrix.

## Example Input File Syntax

!listing /examples/psbt/psbt_axial/psbt_axial.i block=Outputs language=cpp

!syntax parameters /Outputs/QuadSubChannelNormalSliceValues

!syntax inputs /Outputs/QuadSubChannelNormalSliceValues

!syntax children /Outputs/QuadSubChannelNormalSliceValues
