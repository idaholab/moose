# GeneratedMeshGenerator

!syntax description /Mesh/GeneratedMeshGenerator

## Overview

The `GeneratedMeshGenerator` object is the built-in mesh generation capable of creating lines, rectangles, and rectangular
prisms ("boxes"). The mesh automatically creates boundaries that are logically named and numbered as follows:

- In 1D, left = 0, right = 1
- In 2D, bottom = 0, right = 1, top = 2, left = 3
- In 3D, back = 0, bottom = 1, right = 2, top = 3, left = 4, front = 5

The length, width, and height of the domain, as well as the number of elements in each direction can be specified
independently.

!syntax parameters /Mesh/GeneratedMeshGenerator

!syntax inputs /Mesh/GeneratedMeshGenerator

!syntax children /Mesh/GeneratedMeshGenerator
