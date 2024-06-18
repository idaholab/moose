# ExamplePatchMeshGenerator

## Overview

This MeshGenerator object allows the user to create 2D or 3D patch meshes.  The
2D patch mesh has one interior element and four exterior elements, and the 3D
patch test has one interior element and six exterior elements.  Each element in
a patch mesh has a unique shape.  See [!cite](macneal1985patch) for details on
these meshes.

The resulting mesh has sidesets named left, right, top, and bottom, and if a 3D
mesh, sidesets named front and back.

!syntax parameters /Mesh/ExamplePatchMeshGenerator

!syntax inputs /Mesh/ExamplePatchMeshGenerator

!syntax children /Mesh/ExamplePatchMeshGenerator

!bibtex bibliography
