# Peridynamic Generated Mesh

## Description

Mesh `GeneratedMeshPD` object is the built-in mesh generation capable of creating rectangles (2D), and rectangular
prisms ("boxes") (3D) for peridynamic analysis. The mesh automatically creates material points sets that are logically named and numbered as follows:

* In 2D, Left = 0, Right = 1, Bottom = 2, Top = 3

* In 3D, Back = 0, Front = 1, Left = 2, Right = 3, Bottom = 4, Top = 5

The length, width, and height of the domain, as well as the number of elements in each direction can be specified
independently.

!syntax parameters /Mesh/GeneratedMeshPD

!syntax inputs /Mesh/GeneratedMeshPD

!syntax children /Mesh/GeneratedMeshPD
