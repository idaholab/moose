# CartesianMeshGenerator

!syntax description /Mesh/CartesianMeshGenerator

## Overview

The `CartesianMeshGenerator` object is the built-in mesh generation capable of creating lines, rectangles, and hexahedra ("boxes").
The mesh spacing can be non-uniform and each line/rectangle/hexahedron can be assigned a separate subdomain id.
The mesh automatically creates side sets that are logically named and numbered as follows:

- In 1D, left = 0, right = 1
- In 2D, bottom = 0, right = 1, top = 2, left = 3
- In 3D, back = 0, bottom = 1, right = 2, top = 3, left = 4, front = 5

The length, width, and height of each element, as well as their subdomain id can be set independently.
Each linear subdivision in x, y, or z can be additionally subdivided into sub-elements.

## Example Syntax

!listing test/tests/meshgenerators/cartesian_mesh_generator/cartesian_mesh_3D.i
         block=Mesh

!syntax parameters /Mesh/CartesianMeshGenerator

!syntax inputs /Mesh/CartesianMeshGenerator

!syntax children /Mesh/CartesianMeshGenerator
