# MeshCut2DFractureUserObject

!syntax description /UserObjects/MeshCut2DFractureUserObject

## Overview

This class is used to define an evolving cutting plane for 2D XFEM simulations based on a mesh that defines the initial crack and uses fracture integrals or maximum stress to grow that crack. It (1) reads in a mesh describing the crack surface or uses nucleated cracks, (2) uses the mesh to do initial cutting of 2D elements, and (3) grows the mesh incrementally based on fracture domain integrals to allow nonplanar crack growth based on propagation directions determined by fracture integrals.  A maximum stress criterion can also be used to drive crack growth.

The crack propagates if the failure criterion is met, given by:

\begin{equation}
K_c \le \sqrt{K^2_I+K^2_{II}}
\end{equation}

where $K_I$ and $K_{II}$ are the mode I and II stress intensity factors provided by the fracture integral and the material property $K_c$ is defined in the input file with a constant value for the entire domain using [!param](/UserObjects/MeshCut2DFractureUserObject/k_critical) or from a [CrackFrontNonlocalScalarMaterial.md] VectorPostprocessor that samples a material property at each crack front point and is specified by [!param](/UserObjects/MeshCut2DFractureUserObject/k_critical_vectorpostprocessor) and [!param](/UserObjects/MeshCut2DFractureUserObject/k_critical_vector_name).  The crack growth direction is given by the direction that maximizes the crack-tip hoop stress, given by Equation 5 in [!cite](jiang2020).  The growth increment is a user provided input given by [!param](/UserObjects/MeshCut2DFractureUserObject/growth_increment).  The fracture integrals $K_I$ and $K_{II}$ are obtained from the [InteractionIntegral.md] vectorpostprocessor specified in the input file by [!param](/UserObjects/MeshCut2DFractureUserObject/ki_vectorpostprocessor) and [!param](/UserObjects/MeshCut2DFractureUserObject/kii_vectorpostprocessor).  Defaults for [!param](/UserObjects/MeshCut2DFractureUserObject/ki_vectorpostprocessor) and [!param](/UserObjects/MeshCut2DFractureUserObject/kii_vectorpostprocessor) use the names produced by the [/DomainIntegralAction.md] which is the standard way to set-up the [InteractionIntegral.md].

Near a free surface, the integration volumes of the rings used to compute the `InteractionIntegral` will intersect the surface, leading to a reduction in the fracture integral values.  This can lead to cracks becoming unable to farther propagate as they approach free surfaces.  For these cases, a maximum stress criterion computed using [CrackFrontNonlocalStress.md] vectorpostprocessor can be used for crack growth using an additional failure criterion given by:

\begin{equation}
\sigma_c \le \sigma_{nn}
\end{equation}

where $\sigma_c$ is a critcal stress normal to the crack face specified by [!param](/UserObjects/MeshCut2DFractureUserObject/stress_threshold) and $\sigma_{nn}$ is the average scalar stress measure normal to the crack face specified by [!param](/UserObjects/MeshCut2DFractureUserObject/stress_vectorpostprocessor).  The crack front normal stress will only extend the crack in the direction it is already going and will not cause the crack to curve.

Quasistatic behavior is assumed, and an iterative approach is taken to repeatedly solve the equilibrium equations, evaluate the fracture integrals, and as indicated by the failure criterion, incrementally advance the crack until the failure criterion is no longer met. To iteratively repeat the solution in this manner for each step, [!param](/Executioner/Steady/max_xfem_update) must be set in the `Executioner` block, and should be large enough to allow a sufficient number of iterations for crack growth to cease during each timestep.

## Example Input Syntax

The following input file provides an example of a `MeshCut2DFractureUserObject` that uses fracture integrals to propagate the crack.  In this example, crack growth stops before the crack grows through the right boundary due to the fracture integral q-function intersecting the boundary.  A stress based crack growth criterion is used in conjunction with the fracture integrals by uncommenting the two lines containing `stress_vectorpostprocessor` and `stress_threshold` in the `MeshCut2DFractureUserObject`.  The stress based growth criterion causes the crack to continue to grow through the right boundary, as expected.

!listing test/tests/mesh_cut_2D_fracture/kcrit_stress_based_meshCut_uo.i block=UserObjects

!syntax parameters /UserObjects/MeshCut2DFractureUserObject

!syntax inputs /UserObjects/MeshCut2DFractureUserObject

!syntax children /UserObjects/MeshCut2DFractureUserObject
