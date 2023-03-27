# MeshCut2DFractureUserObject

!syntax description /UserObjects/MeshCut2DFractureUserObject

## Overview

This class: (1) reads in a mesh describing the crack surface, (2) uses the mesh to do initial cutting of 2D elements, and (3) grows the mesh incrementally based on the domain integral methods to allow nonplanar crack growth based on empirical propagation direction.  The crack will propagate if the failure criterion is met, given by:
\begin{equation}
K_c \le \sqrt{K^2_I+K^2_{II}}
\end{equation}
where $K_I$ and $K_{II}$ are the mode I and II stress intensity factors provided by the domain integral and the material property $K_c$ is given defined in the input file by [!param](/UserObjects/MeshCut2DFractureUserObject/k_critical).  The crack growth direction is given by the direction the maximized the crack-tip hoop stress, given by equation 5 in the the journal article [!cite](jiang2020).  The growth amount is a user provided input given in the defined by [!param](/UserObjects/MeshCut2DFractureUserObject/growth_increment).  The growth rate is not defined and the crack will grow until the failure criterion is no longer met which is valid for quasi-static loading.  For this to work correctly, [!param](/Executioner/Steady/max_xfem_update) set in the `Executioner` block should be large enough for crack growth to cease during each timestep.

## Example Input Syntax

!listing test/tests/solid_mechanics_basic/edge_crack_2d_propagation_mhs.i block=UserObjects

!syntax parameters /UserObjects/MeshCut2DFractureUserObject

!syntax inputs /UserObjects/MeshCut2DFractureUserObject

!syntax children /UserObjects/MeshCut2DFractureUserObject
