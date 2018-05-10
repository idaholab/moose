# LineSegmentLevelSetAux

!syntax description /AuxKernels/LineSegmentLevelSetAux

## Description

The `LineSegmentLevelSetAux` calculates level set values for an interface that is defined by a set of line segments(`LineSegmentCutSetUserObject`). Usually the value of level set at a given point is calculated as a signed distance of this point from the boundary of the interface, with the sign determined by whether x is inside the closed interface contour.    

## Example Input File Syntax

!listing modules/xfem/test/tests/moving_interface/phase_transition.i block=AuxKernels/ls

!syntax parameters /AuxKernels/LineSegmentLevelSetAux

!syntax inputs /AuxKernels/LineSegmentLevelSetAux

!syntax children /AuxKernels/LineSegmentLevelSetAux

!bibtex bibliography
