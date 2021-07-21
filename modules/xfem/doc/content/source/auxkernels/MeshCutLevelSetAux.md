# MeshCutLevelSetAux

!syntax description /AuxKernels/MeshCutLevelSetAux

## Description

The `MeshCutLevelSetAux` calculates level set values for an interface that is defined by a cutter mesh([InterfaceMeshCut2DUserObject](InterfaceMeshCut2DUserObject.md) or [InterfaceMeshCut3DUserObject](InterfaceMeshCut3DUserObject.md)). The value of level set at a given point is calculated as a signed distance of this point from the surface of the interface, with the sign determined by whether this point is inside the closed interface contour.   

## Example Input File Syntax

!listing modules/xfem/test/tests/moving_interface/phase_transition_2d.i block=AuxKernels/ls

!syntax parameters /AuxKernels/MeshCutLevelSetAux

!syntax inputs /AuxKernels/MeshCutLevelSetAux

!syntax children /AuxKernels/MeshCutLevelSetAux

!bibtex bibliography
