# CrackMeshCut3DUserObject

!syntax description /UserObjects/CrackMeshCut3DUserObject

## Overview

This class: (1) reads in a mesh describing the crack surface, (2) uses the mesh to do initial cutting of 3D elements, and (3) grows the mesh incrementally based on prescribed growth functions. The code is interfaced with domain integral methods to allow nonplanar crack growth based on empirical propagation direction and speed laws.

## Example Input Syntax

!listing test/tests/solid_mechanics_basic/edge_crack_3d_propagation.i block=UserObjects

!syntax parameters /UserObjects/CrackMeshCut3DUserObject

!syntax inputs /UserObjects/CrackMeshCut3DUserObject

!syntax children /UserObjects/CrackMeshCut3DUserObject
