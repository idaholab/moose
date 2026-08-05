# XFEMCutMeshOutput

!syntax description /Outputs/XFEMCutMeshOutput

## Overview

This class outputs the lower-dimensional mesh used to define cuts with XFEM.  It outputs the mesh in the Exodus format and outputs it in the original (undeformed) configuration.  The `XFEMCutMeshOutput` creates a file using the naming convention [!param](/Outputs/XFEMCutMeshOutput/file_base)_XFEMCutMeshOutput.e.  This class outputs a new Exodus file at every timestep with an extension that matches the convention used by MOOSE for other modified meshes (i.e. *.e-s0001, *.e-s0002, etc.),  because it assumes that the cutting mesh is always adding new nodes and elements as the crack grows.  This class is currently only implemented to be operable with the MeshCut user objects ([CrackMeshCut3DUserObject.md], [MeshCut2DFractureUserObject.md] and [MeshCut2DFunctionUserObject.md]) but will eventually include all cutter user objects deriving off a GeometricCutUserObject.

## Diagnosing XFEM Cutting Errors with `execute_on = xfem_mark`

When an XFEM run aborts inside the EFA cut-marking / fragment update phase (for example with errors such as `EFAelement3D::getMasterInfo, cannot find the given EFANode`), the failure happens *before* the timestep's normal output is written, so the standard `final` or `timestep_end` Exodus snapshot does not capture the state of the cutter mesh that caused the error.

Setting [!param](/Outputs/XFEMCutMeshOutput/execute_on) to `xfem_mark` on the `XFEMCutMeshOutput` writes the cutter mesh at the XFEM marking phase, which runs *before* the fragment update.  The resulting Exodus file therefore reflects the cutter geometry that XFEM is about to apply.  Viewing this file in ParaView lets you see exactly where the crack-growth surface is at the moment of the abort -- often the geometry reveals overlapping cut surfaces, near-degenerate triangles, or an unexpected crack-front location that drove the failure.

This output is purely diagnostic; remove or revert to `final` once the underlying input is fixed, since it produces an additional Exodus file per cut iteration.

## Example Input Syntax

!listing test/tests/solid_mechanics_basic/face_crack_3d_xfem_scc.i block=Outputs replace=['execute_on = final','execute_on = xfem_mark']

!syntax parameters /Outputs/XFEMCutMeshOutput

!syntax inputs /Outputs/XFEMCutMeshOutput

!syntax children /Outputs/XFEMCutMeshOutput
