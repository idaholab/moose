# MFEMMeshGeneratorMesh

!if! function=hasCapability('mfem')

## Overview

`MFEMMeshGeneratorMesh` receives the final mesh produced by a chain of MFEM mesh generators, such
as [MFEMGeneratedMeshGenerator.md], and constructs the `mfem::ParMesh` used by an [MFEMProblem.md].

MOOSE selects `MFEMMeshGeneratorMesh` automatically when the `[Mesh]` block contains an MFEM mesh
generator and no mesh `type` is specified. Users normally do not need to select this mesh type
directly. The final generator in the chain must be an MFEM mesh generator; libMesh and MFEM mesh
generators cannot be mixed in the same chain.

As MOOSE checks for a libMesh mesh during setup, `MFEMMeshGeneratorMesh` also builds a small
libMesh placeholder. The placeholder is not the simulation mesh; MFEM objects access the generated
`mfem::ParMesh` through the `getMFEMParMesh()` accessor.

## Example Input File Syntax

!listing test/tests/mfem/meshgenerators/generated/test.i block=Mesh Problem

!syntax parameters /Mesh/MFEMMeshGeneratorMesh

!syntax inputs /Mesh/MFEMMeshGeneratorMesh

!syntax children /Mesh/MFEMMeshGeneratorMesh

!if-end!

!else
!include mfem/mfem_warning.md
