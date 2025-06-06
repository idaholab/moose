# QuadSubChannelNormalSliceValues

!syntax description /Outputs/QuadSubChannelNormalSliceValues

## Overview

<!-- -->

This kernel is used to save the solution variables in a file, for the square subchannel problem.
Imagine a plane perpendicular to the flow direction at a specific [!param](/Outputs/QuadSubChannelNormalSliceValues/height) defined by the user.
The solution [!param](/Outputs/QuadSubChannelNormalSliceValues/variable) defined by the user is interpolated at the intersection of that plane
and the subchannel mesh and printed on that [!param](/Outputs/QuadSubChannelNormalSliceValues/file_base) as a matrix.

## Example Input File Syntax

!listing /validation/PNNL_12_pin/steady_state/2X6_ss.i block=Outputs language=moose

!syntax parameters /Outputs/QuadSubChannelNormalSliceValues

!syntax inputs /Outputs/QuadSubChannelNormalSliceValues

!syntax children /Outputs/QuadSubChannelNormalSliceValues
