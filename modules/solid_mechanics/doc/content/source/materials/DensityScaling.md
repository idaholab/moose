# Density Scaling

!syntax description /Materials/DensityScaling

## Description

The material `DensityScaling` computes the extra element density required
in every element to allow explicit dynamics solves to be solved with a time
step prescribed by a user (see [CriticalTimeStep](/CriticalTimeStep.md)).
The addition of mass can alter the dynamics of the system, so usage of mass
scaling is particularly recommended when the finite element mesh contains a
handful of small finite elements or when system fundamental frequencies do not
have a key influence on the simulation output metrics.

## Example Input File

!listing modules/tensor_mechanics/test/tests/central_difference/consistent/3D/3d_consistent_explicit_mass_scaling.i block=Materials/density_scaling

!syntax parameters /Materials/DensityScaling

!syntax inputs /Materials/DensityScaling

!syntax children /Materials/DensityScaling

!bibtex bibliography
