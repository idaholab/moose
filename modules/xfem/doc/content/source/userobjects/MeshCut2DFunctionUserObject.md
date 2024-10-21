# MeshCut2DFunctionUserObject

!syntax description /UserObjects/MeshCut2DFunctionUserObject

## Overview

This class is used to define an evolving cutting plane for 2D XFEM simulations based on a mesh that defines an initial crack, and a user-defined function for growth of that crack. It (1) reads in a mesh describing the crack surface, (2) uses the mesh to do initial cutting of 2D elements, and (3) grows the mesh incrementally based on prescribed growth functions defining the direction and growth rate.

## Example Input Syntax

!listing test/tests/mesh_cut_2D_fracture/crack_front_stress_function_growth.i block=UserObjects

!syntax parameters /UserObjects/MeshCut2DFunctionUserObject

!syntax inputs /UserObjects/MeshCut2DFunctionUserObject

!syntax children /UserObjects/MeshCut2DFunctionUserObject
