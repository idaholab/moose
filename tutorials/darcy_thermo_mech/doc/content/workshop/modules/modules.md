# Modules

!---

## Chemical Reactions

The chemical reactions module provides a set of tools for the calculation of multicomponent aqueous
reactive transport in porous media, originally developed as the MOOSE application RAT
[!citep](guo2013).

!---

## Contact

The MOOSE contact module provides the necessary tools for modeling mechanical contact using
algorithms to enforce constraints between surfaces in the mesh, to prevent penetration and develop
contact forces.

!---

## External PETSc Solver

The External PETSc Solver module provides support for stand-alone native PETSc applications that
to be coupled with moose-based applications.

!---

## Fluid Properties

The Fluid Properties module provides a consistent interface to fluid properties such as density,
viscosity, enthalpy and many others, as well as derivatives with respect to the primary
variables. The consistent interface allows different fluids to be used in an input file by simply
swapping the name of the Fluid Properties UserObject in a plug-and-play manner.

!---

## Functional Expansion Tools

A MOOSE module for continuous, mesh-agnostic, high-fidelity, reduced-data MultiApp coupling

Functional expansions (FXs) are a methodology that represent information as moments of a functional
series [!citep](Flusser2016). This is is related to a Fourier series representation of cyclic
data. Moments are generated via numerical integration for each term in the functional series to
represent the field of interest. These moments can then be used to reconstruct the field in a
separate app [!citep](Wendt2018a,Wendt2017c,Kerby2017).

!---

## Heat Conduction

Basic utilities for solving the transient heat conduction equation:

!equation
\rho c_p \pf{T}{t} - \nabla\cdot k \nabla T - s = 0

!---

## Level Set

The level set module provides basic functionality to solve the level set equation, which is
simply the multi-dimensional advection equation:

!equation
\pf{u}{t} + \vec{v}\cdot\nabla u = 0

!---

## Navier Stokes

The MOOSE Navier-Stokes module is a library for the implementation of simulation tools that solve the
Navier-Stokes equations using the continuous Galerkin finite element (CGFE) method. The Navier-Stokes
equations are usually solved using either the pressure-based, incompressible formulation (assuming a
constant fluid density), or the density-based, compressible formulation.

!---

## Phase Field

The MOOSE phase field module is a library for simplifying the implementation of simulation tools that
employ the phase field model.

!---

## Porous Flow

The PorousFlow module is a library of physics for fluid and heat flow in porous media. It is
formulated in an extremely general manner, so is capable of solving problems with an arbitrary number
of phases (gas, liquid, etc) and fluid components (species present in each fluid phase), using any
set of primary variables.

!---

## Reconstructed Discontinuous Galerkin (rDG)

The MOOSE rDG module is a library for the implementation of simulation tools that solve
convection-dominated problems using the class of so-called reconstructed discontinuous Galerkin (rDG)
methods. The specific rDG method implemented in this module is rDG(P0P1), which is equivalent to the
second-order cell-centered finite volume method (FVM).

!---

## Reactor

The Reactor module adds advanced meshing capabilities to MOOSE so that users can create complex-geometry
meshes related to the structures of reactor cores. This includes objects for creating and modifying
hexagonal mesh components for assemblies, stitching assemblies together to form core meshes, creating
peripheral regions for assemblies and cores, adding IDs for pins and assembly regions, and enabling
the dynamic and static simulation of rotational control drums.

!---

## Stochastic Tools

The stochastic tools module is a toolbox designed for performing stochastic analysis for MOOSE-based
applications.

!---

## Tensor Mechanics

The Tensor Mechanics module is a library of simulation tools that solve continuum mechanics
problems. The module can be used to simulation both linear and finite strain
mechanics, including Elasticity and Cosserat elasticity, Plasticity and micromechanics plasticity,
Creep, and Damage due to cracking and property degradation.

!---

## Thermal Hydraulics

The Thermal Hydraulics module is a library of components that can be used to build thermal-hydraulic
simulations. Basic capabilities include a 1-phase, variable-area, inviscid, compressible flow model
with a non-condensable vapor mixture, 2-D and 3-D heat conduction, a control logic system, and
pluggable closure systems and models.

!---

## Extended Finite Element Method (XFEM)

A MOOSE-based implementation of the extended finite element method, which is a numerical method that
is especially designed for treating discontinuities.
