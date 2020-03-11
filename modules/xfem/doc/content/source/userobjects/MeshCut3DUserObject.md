# MeshCut3DUserObject

!syntax description /UserObjects/MeshCut3DUserObject

## Overview

This class: (1) reads in a mesh describing the crack surface, (2) uses the mesh to do initial cutting of 3D elements, and (3) grows the mesh incrementally based on prescribed growth functions. Interfacing with the domain integral methods is needed to grow the surface mesh based on physically determined propagation directions and speeds.

## Example Input Syntax

!listing test/tests/solid_mechanics_basic/edge_crack_3d_propagation.i block=UserObjects

!syntax parameters /UserObjects/MeshCut3DUserObject

!syntax inputs /UserObjects/MeshCut3DUserObject

!syntax children /UserObjects/MeshCut3DUserObject
