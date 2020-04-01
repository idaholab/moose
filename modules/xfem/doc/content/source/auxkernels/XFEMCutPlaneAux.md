# XFEMCutPlaneAux

!syntax description /AuxKernels/XFEMCutPlaneAux

# Description

The MOOSE XFEM implementation uses the phantom node technique, in which elements traversed by a discontinuity are split into two partial elements, each containing physical and non-physical material. The `XFEMCutPlaneAux` output is used in [XFEM Paraview Plugin](https://github.com/idaholab/XFEMParaviewPlugin) (`MooseXfemClip`) to define a cutting plane to cut off the non-physical portions of partial elements. It can be automatically added when `output_cut_plane` = `true` in [XFEM](syntax/XFEM/index.md) block.
