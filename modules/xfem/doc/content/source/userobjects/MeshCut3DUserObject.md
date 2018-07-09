# MeshCut3DUserObject

!syntax description /UserObjects/MeshCut3DUserObject

## Description

This class: (1) reads in a mesh describing the crack surface, (2) uses the mesh to do initial cutting of 3D elements, and (3) grows the mesh based on prescribed growth functions. We can't currently incrementally grow the crack in 3D. Interfacing with the domain integral methods is needed to grow the surface mesh based on physically determined propagation directions and speeds.

!syntax parameters /UserObjects/MeshCut3DUserObject

!syntax inputs /UserObjects/MeshCut3DUserObject

!syntax children /UserObjects/MeshCut3DUserObject
