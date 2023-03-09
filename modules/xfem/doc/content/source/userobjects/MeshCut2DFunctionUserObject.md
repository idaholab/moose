# MeshCut2DFunctionUserObject

!syntax description /UserObjects/MeshCut2DFunctionUserObject

## Overview

This class: (1) reads in a mesh describing the crack surface, (2) uses the mesh to do initial cutting of 2D elements, and (3) grows the mesh incrementally based on prescribed growth functions defining the direction and growth speed.

## Example Input Syntax

!listing test/tests/solid_mechanics_basic/edge_crack_2d_propagation.i block=UserObjects

!syntax parameters /UserObjects/MeshCut2DFunctionUserObject

!syntax inputs /UserObjects/MeshCut2DFunctionUserObject

!syntax children /UserObjects/MeshCut2DFunctionUserObject
