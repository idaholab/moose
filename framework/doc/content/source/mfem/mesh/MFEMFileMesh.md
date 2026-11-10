# MFEMFileMesh

!if! function=hasCapability('mfem')

## Overview

`MFEMFileMesh` reads an `mfem::ParMesh` from file for use in an [MFEMProblem.md]. Exodus files are
supported, along with other mesh formats listed [here](https://mfem.org/mesh-formats/).

## Example Input File Syntax

!listing test/tests/mfem/kernels/diffusion.i block=Problem Mesh

!syntax parameters /Mesh/MFEMFileMesh

!syntax inputs /Mesh/MFEMFileMesh

!syntax children /Mesh/MFEMFileMesh

!if-end!

!else
!include mfem/mfem_warning.md
