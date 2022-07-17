# Modules

!---

## Chemical Reactions

Provides a set of tools for the calculation of multicomponent aqueous
reactive transport in porous media, originally developed as the MOOSE application RAT
[!citep](guo2013).

!---

## Contact

Provides the necessary tools for modeling mechanical contact using
algorithms to enforce constraints between surfaces in the mesh, to prevent penetration and develop
contact forces.

!---

## Contact: Frictional Ironing Problem

!media contact/ironing_gallery.mp4
  style=margin:auto;
  caption=Frictional ironing model using mortar contact.

!---

## Electromagnetics

The Electromagnetics module provides components and models to simulate electromagnetic wave problems
using MOOSE, and facilitate coupling of electromagnetic simulations to other physical domains.
Maxwell's equations are solved for complex fields using a Helmholtz wave equation formulation in 1-D
and 2-D. Electrostatic contact is also provided for imperfect electric interfaces.

!media gallery/dipole_antenna.mp4
  style=width:40%;margin:auto;
  caption=Electric field radiation pattern of half-wave dipole antenna.

!---

## External PETSc Solver

Provides support for stand-alone native PETSc applications that
are to be coupled with moose-based applications. It is also a general
example for coupling to an external application.

!---

## Fluid Properties

The Fluid Properties module provides a consistent interface to fluid properties such as density,
viscosity, enthalpy and many others, as well as derivatives with respect to the primary
variables. The consistent interface allows different fluids to be used in an input file by simply
swapping the name of the Fluid Properties UserObject in a plug-and-play manner.

!---

## Fluid-Structure Interaction

Provides tools that solve fluid and structure problems, wherein, their behavior is
inter-dependent. Currently capable of simulating fluid-structure interaction
behavior using an acoustic formulation for the fluid.

!---

## Functional Expansion Tools

A MOOSE module for continuous, mesh-agnostic, high-fidelity, reduced-data MultiApp coupling

Functional expansions (FXs) are a methodology that represent information as moments of a functional
series [!citep](Flusser2016). This is is related to a Fourier series representation of cyclic
data. Moments are generated via numerical integration for each term in the functional series to
represent the field of interest. These moments can then be used to reconstruct the field in a
separate app [!citep](Wendt2018a,Wendt2017c,Kerby2017).

!---

## Geochemistry

Solves geochemical models. The capabilities include:

- Equilibrium aqueous systems
- Redox disequilibrium
- Sorption and surface complexation
- Kinetics
- All of the above combined with fluid and heat transport

It is designed to interface easily with the porous flow module so that complicated reactive transport scenarios can be studied.

!--

## Heat Conduction

Basic utilities for solving the transient heat conduction equation:

!equation
\rho c_p \pf{T}{t} - \nabla\cdot k \nabla T - s = 0

Also contains capability for generalized heat transfer (convection, radiation, ...). Will likely be renamed
heat transfer in the future, accordingly.

!---

## Level Set

The level set module provides basic functionality to solve the level set equation, which is
simply the multi-dimensional advection equation:

!equation
\pf{u}{t} + \vec{v}\cdot\nabla u = 0

!---

## Navier Stokes

A library for the implementation of simulation tools that solve the multi-dimensional
Navier-Stokes equations using either the continuous Galerkin finite element (CGFE) or the finite volume (FV) method. The Navier-Stokes equations may be solved with:

- An incompressible formulation (CGFE & FV)
- A weakly compressible formulation (FV)
- A fully compressible formulation (FV)

Zero-dimensional turbulence models are available and coarse regularized k-epsilon will be added soon.

!---

## Navier Stokes

Flow in a lid-driven cavity with Re=417 (left) and Re=833 (right).

!row!
!col! width=50%

!media darcy_thermo_mech/ns-re-417.png style=width:80%;background:white;

!col-end!

!col! width=50%

!media darcy_thermo_mech/ns-re-833.png style=width:82%;background:white;

!col-end!
!row-end!

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

## Ray Tracing

Provides capability for tracing rays through a finite element mesh. Notable features include:

- Contribution to residuals and Jacobians from along a ray
- Ray interaction with internal and external boundaries
- Supports storage and manipulation of data unique to each ray
- Supports ray interaction with field variables
- Highly parallelizable: tested to 20k MPI ranks

!---

## Ray Tracing: Flashlight source

!media ray_tracing/cone_ray_study_u.png
  style=width:49%;float:left;
  caption=Example of flashlight point sources within a diffusion-reaction problem

!media ray_tracing/cone_ray_study_rays.png
  style=width:49%;float:right;
  caption=Overlay of the rays used within the problem on the left

!--

## Reconstructed Discontinuous Galerkin (rDG)

The MOOSE rDG module is a library for the implementation of simulation tools that solve
convection-dominated problems using the class of so-called reconstructed discontinuous Galerkin (rDG)
methods. The specific rDG method implemented in this module is rDG(P0P1), which is equivalent to the
second-order cell-centered finite volume method (FVM).

!---

## Reactor

Adds advanced meshing capabilities to MOOSE so that users can create complex-geometry
meshes related to the structures of reactor cores. This includes:

- Creating and modifying hexagonal mesh components for assemblies
- Stitching assemblies together to form core meshes
- Creating peripheral regions for assemblies and cores
- Adding IDs for pins and assembly regions
- Enabling the dynamic and static simulation of rotational control drums.

!---

## Reactor: Meshing a Microreactor

!media tutorials/darcy_thermo_mech/reactor_microreactor.png
  caption=Meshing a microreactor in stages using the Reactor module: from pins and control drums to assemblies, cores, and peripheral regions.

!---

## Stochastic Tools

!row!
!col! width=60%

A toolbox designed for performing stochastic analysis for MOOSE-based
applications. Capabilities include:

- Parameter Studies
- Sensitivity Analysis
- Uncertainty Quantification
- Surrogate/Reduced-Order Model generation

!col-end!

!col! width=40%

!media darcy_thermo_mech/stochastic_tools.png style=width:100%;background:white;

!col-end!
!row-end!

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
