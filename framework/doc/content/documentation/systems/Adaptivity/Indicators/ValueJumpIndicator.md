# ValueJumpIndicator

!syntax description /Adaptivity/Indicators/ValueJumpIndicator

## Description

`ValueJumpIndicator` is an error indicator which is appropriate for use with
discontinuous finite element discretizations. Similar to the `GradientJumpIndicator`,
it estimates the error on a given element by integrating the jump in the
chosen variable's value across the boundary with each of its face neighbors.

!syntax parameters /Adaptivity/Indicators/ValueJumpIndicator

!syntax inputs /Adaptivity/Indicators/ValueJumpIndicator

!syntax children /Adaptivity/Indicators/ValueJumpIndicator
