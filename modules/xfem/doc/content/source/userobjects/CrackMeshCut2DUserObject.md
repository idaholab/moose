# CrackMeshCut2DUserObject

!syntax description /UserObjects/CrackMeshCut2DUserObject

## Overview

This class: (1) reads in a mesh describing the crack surface, (2) uses the mesh to do initial cutting of 2D elements, and (3) grows the mesh incrementally based on prescribed growth functions. Future work will interface this userObject with the domain integral methods to allow nonplanar crack growth based on empirical propagation direction and speed laws.

## Example Input Syntax

!listing test/tests/solid_mechanics_basic/edge_crack_2d_propagation.i block=UserObjects

!syntax parameters /UserObjects/CrackMeshCut2DUserObject

!syntax inputs /UserObjects/CrackMeshCut2DUserObject

!syntax children /UserObjects/CrackMeshCut2DUserObject
