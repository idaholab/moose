# QuadSubChannelNormalSliceValues

!syntax description /Outputs/QuadSubChannelNormalSliceValues

## Overview

<!-- -->

This kernel is used to save the solution variables in a file, for the square subchannel problem.
Imagine a plane perpendicular to the flow direction at a specific [!param](/Outputs/QuadSubChannelNormalSliceValues/height) defined by the user.
The solution [!param](/Outputs/QuadSubChannelNormalSliceValues/variable) defined by the user is interpolated at the intersection of that plane
and the subchannel mesh and printed on that [!param](/Outputs/QuadSubChannelNormalSliceValues/file_base) as a matrix.

## Example Input File Syntax

!listing /examples/psbt/psbt_axial/psbt_axial.i block=Outputs language=cpp

!syntax parameters /Outputs/QuadSubChannelNormalSliceValues

!syntax inputs /Outputs/QuadSubChannelNormalSliceValues

!syntax children /Outputs/QuadSubChannelNormalSliceValues
