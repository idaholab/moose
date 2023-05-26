# MeshCut2DFractureUserObject

!syntax description /UserObjects/MeshCut2DFractureUserObject

## Overview

This class is used to define an evolving cutting plane for 2D XFEM simulations based on a mesh that defines the initial crack and uses fracture integrals to grow that crack. It (1) reads in a mesh describing the crack surface, (2) uses the mesh to do initial cutting of 2D elements, and (3) grows the mesh incrementally based on fracture domain integrals to allow nonplanar crack growth based on propagation directions also determined by fracture integrals.

The crack propagates if the failure criterion is met, given by:
\begin{equation}
K_c \le \sqrt{K^2_I+K^2_{II}}
\end{equation}
where $K_I$ and $K_{II}$ are the mode I and II stress intensity factors provided by the fracture integral and the material property $K_c$ is given defined in the input file by [!param](/UserObjects/MeshCut2DFractureUserObject/k_critical).  The crack growth direction is given by the direction the maximized the crack-tip hoop stress, given by Equation 5 in [!cite](jiang2020).  The growth increment is a user provided input given in the defined by [!param](/UserObjects/MeshCut2DFractureUserObject/growth_increment).

Quasistatic behavior is assumed, and an iterative approach is taken to repeatedly solve the equilibrium equations, evaluate the fracture integrals, and as indicated by the failure criterion, incrementally advance the crack until the failure criterion is no longer met. To iteratively repeat the solution in this manner for each step, [!param](/Executioner/Steady/max_xfem_update) must be set in the `Executioner` block, and should be large enough to allow a sufficient number of iterations for crack growth to cease during each timestep.

## Example Input Syntax

!listing test/tests/solid_mechanics_basic/edge_crack_2d_propagation_mhs.i block=UserObjects

!syntax parameters /UserObjects/MeshCut2DFractureUserObject

!syntax inputs /UserObjects/MeshCut2DFractureUserObject

!syntax children /UserObjects/MeshCut2DFractureUserObject
