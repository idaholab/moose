# XFEMCutMeshOutput.md

!syntax description /Outputs/XFEMCutMeshOutput

## Overview

This class outputs the XFEM mesh used to cut the geometry.  This class only outputs the mesh in Exodus format and it only outputs the node positions.  This class always outputs a new Exodus file at every timestep with a postfix like *.e-s0001 because it assumes that the cutting mesh is always adding new nodes and elements as it grows.  This class is currently only implemented for the [MeshCut2DFractureUserObject.md] and [MeshCut2DFunctionUserObject.md] user object but will eventually include all cutter user objects deriving off a GeometricCutUserObject.

## Example Input Syntax

!listing test/tests/solid_mechanics_basic/edge_crack_2d_propagation.i block=Outputs

!syntax parameters /Outputs/XFEMCutMeshOutput

!syntax inputs /Outputs/XFEMCutMeshOutput

!syntax children /Outputs/XFEMCutMeshOutput
