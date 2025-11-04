# XFEMCutMeshOutput.md

!syntax description /Outputs/XFEMCutMeshOutput

## Overview

This class outputs the lower-dimensional mesh used to define cuts with XFEM.  It outputs the mesh in the Exodus format and outputs it in the original (undeformed) configuration.  The `XFEMCutMeshOutput` creates a file using the naming convention [!param](/Outputs/XFEMCutMeshOutput/file_base)_XFEMCutMeshOutput.e.  This class outputs a new Exodus file at every timestep with an extension that matches the convention used by MOOSE for other modified meshes (i.e. *.e-s0001, *.e-s0002, etc.),  because it assumes that the cutting mesh is always adding new nodes and elements as the crack grows.  This class is currently only implemented to be operable with the [MeshCut2DFractureUserObject.md] and [MeshCut2DFunctionUserObject.md] user objects but will eventually include all cutter user objects deriving off a GeometricCutUserObject.

## Example Input Syntax

!listing test/tests/mesh_cut_2D_fracture/edge_crack_2d_propagation.i block=Outputs

!syntax parameters /Outputs/XFEMCutMeshOutput

!syntax inputs /Outputs/XFEMCutMeshOutput

!syntax children /Outputs/XFEMCutMeshOutput
