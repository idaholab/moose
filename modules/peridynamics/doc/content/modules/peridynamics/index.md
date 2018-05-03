# Peridynamics Module

!media pd_fuel_fracture_examples.png style=width:380px;padding-left:20px;float:right;
       caption=Peridynamics results of 3D fuel fragmentation

The MOOSE peridynamic module is a library for solving classical solid mechanics and heat transfer problems using the peridynamics theory. For advanced deformation modeling, such as plasticity and creep, the peridynamics module should be used in conjunction with material classes from tensor mechanics module. More information about this module is found below:

- [System Documentation List](peridynamics/systems.md)

## Basic Concepts in Peridynamics Theory

- [Horizon and States](peridynamics/HorizonStates.md): Material points, family and states

- [Deformation Gradients](peridynamics/DeformationGradients.md): Weighted least squares technique based nonlocal deformation gradients

## Peridynamic Models

- [Bond-based Mechanics Models](peridynamics/PeridynamicModels.md): Force state only depends on the bond itself, action and reaction in the opposite direction

- [Ordinary State-based Mechanics Models](peridynamics/PeridynamicModels.md): Force state depends on all bonds within two families, action and reaction in the opposite direction

- [Non-ordinary State-based Mechanics Models](peridynamics/PeridynamicModels.md): Force state depends on all bonds within two families, action and reaction in different directions

- [Bond-based Heat Conduction Models](peridynamics/PeridynamicModels.md): Response function only depends on the bond itself, action and reaction in the opposite direction

- [Coupled Thermo-Mechanical Models](peridynamics/PeridynamicModels.md): Peridynamic mechanics models coupled with peridynamic heat conduction models

## Using Materials From Tensor Mechanics in Correspondence Material Models

In peridynamic correspondence material model, concepts such as strain and stress tensors from classical Continuum Mechanics still apply, i.e., given the nonlocal deformation gradient calculated in peridynamics, calculations based on nonlocal deformation gradient to establish the constitutive relationship between stress and strain follows the same methodology as in local continuum theory. Strain and stress tensors from Continuum Mechanics reside at each discrete material point rather than quadrature point in peridynamic correspondence material model. Plasticity and creep material models from tensor mechanics can be directly used in peridynamics for nonlinear deformation modeling.

- [Strains](tensor_mechanics/Strains.md)

- [Stresses](tensor_mechanics/Stresses.md)

## Spatial Discretization

Similar to mesh generation in finite element methods, a spatial discretization is required to discretize the domain of interest into discrete material points. Family, or connectivity, information needs to be built for each material points. Current peridynamic module supports two type of discretization schemes: build-in regular (i.e., 2D & 3D rectangular) domain discretization and external ExodusII FEM elements based domain discretization.
