# BoundaryShortestDistanceToSurface

!syntax description /UserObjects/BoundaryShortestDistanceToSurface

## Overview

The `BoundaryShortestDistanceToSurface` user object precomputes and caches the geometric shift data required by the Shifted Boundary Method (SBM) and Shifted Interface Method (SIM). At each quadrature point on designated surrogate boundary sides, it provides:
- The **distance vector** ($\vec{d}$) from the surrogate quadrature point to its closest projection on the true surface.
- The **true surface normal** ($\vec{n}$) evaluated at that projected point.

These quantities are used by downstream objects—such as interface kernels and material models—to perform Taylor expansions that transfer boundary/interface conditions from the surrogate boundary to the true interface.

## Distance Data Sources

Distance and normal data can be supplied through two modes:
1. **Analytical Distance Functions**: Through the [!param](/UserObjects/BoundaryShortestDistanceToSurface/surfaces) parameter, specifying parsed functions or distance functions corresponding to each surrogate boundary in [!param](/UserObjects/BoundaryShortestDistanceToSurface/boundary).
2. **Mesh-Based Interfaces via Manager**: Through the [!param](/UserObjects/BoundaryShortestDistanceToSurface/manager) parameter, referencing an [`SBMInterfaceManager`](SBMInterfaceManager.md) and specifying the subdomain pairs ([!param](/UserObjects/BoundaryShortestDistanceToSurface/interface_subdomain_pairs)) that correspond to each boundary.

The [Shifted Cohesive Zone Physics](syntax/Physics/SolidMechanics/ShiftedCohesiveZone/index.md) can configure and instantiate this object automatically.

## Element Scale Validation and Warnings

In the Shifted Boundary Method, the surrogate boundary is expected to be adjacent to the true interface (lying within approximately one element layer). To prevent silently computing inaccurate Taylor expansions when an incorrect surface or boundary has been specified:
- The object computes the estimated element diagonal $\sqrt{\text{dim}} \cdot V^{1/\text{dim}}$ (where $V$ is the element volume).
- If the shift distance $\|\vec{d}\|$ exceeds this estimated element diagonal, a warning is emitted reporting the element ID, side index, distance magnitude, and element diagonal scale.
- If large shift distances are intentional for a specific model, this warning can be disabled by setting [!param](/UserObjects/BoundaryShortestDistanceToSurface/suppress_distance_warning) to `true`.

## Example Input File Syntax

The following snippet demonstrates configuring `BoundaryShortestDistanceToSurface` on an internal surrogate interface using an analytical distance function:

!listing modules/shifted_boundary_method/test/tests/shifted_cohesive_zone/angled_sczm_common.i block=UserObjects/sbm_distance_uo

!syntax parameters /UserObjects/BoundaryShortestDistanceToSurface

!syntax inputs /UserObjects/BoundaryShortestDistanceToSurface

!syntax children /UserObjects/BoundaryShortestDistanceToSurface
