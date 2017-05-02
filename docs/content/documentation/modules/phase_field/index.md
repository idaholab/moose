# Phase Field Module

!media docs/media/phase_field/pf_module_examples_vertical.png width=200px padding-left=20px float=right caption=Phase field results produced using the MOOSE Phase Field Module

* [Systems](phase_field/systems.md)

The MOOSE phase field module is a library for simplifying the implementation of simulation tools that employ the phase field model. Multiphysics capability that includes mechanics and heat conduction can be obtained by employing the _tensor mechanics_ and _heat conduction_ modules. More information about this module is found below:

## Basic Phase Field Model Information
* [Basic Phase Field Equations](phase_field/Phase Field Equations.md) - Basic information about the equations underlying the phase field module
* [Expression Builder](Function Materials/ExpressionBuilder.md) - Using automatic differentiation of free energy material objects
* [Solving Phase Field Models](phase_field/Solving.md) - Basic info about solving phase field models
* [Function Material Kernels](phase_field/FunctionMaterialKernels.md) - Working with Function Materials that carry around their own derivatives
* Phase Field Model Units - Discussion of units in phase field models
* Anisotropy - Support of anisotropic mobilities and interfacial energies
* CALPHAD - Using thermodynamic databases to parameterize phase field models
* Quantitative Two Component Polynomial Free Energies - Simple two component models using polynomial free energies
* [FAQ](phase_field/FAQ.md) - Frequently asked questions about the phase field modules

## Multiple Phase Models
MOOSE provides capabilities that enable the easy development of multiphase field model. A free energy expression has to be provided for each individual phase. Different systems exist to combine those _phase free energies_ into a _global free energy_.

* [Two-phase Models](Multi Phase/WBM Two Phase.md) - Two phases, one phase order parameter
* [Kim-Kim-Suzuki Model](Multi Phase/KKS.md) - per-phase concentrations, two phases
* [Multiphase Models](Multi Phase/WBM.md) - _N_ phases, _N_ phase order parameters
* Grand Potential Model - solving a Legendre transform of teh phase field equations, where the independent variable is the chemical potential

!media docs/media/phase_field/solutionrasterizer.png width=200px padding-left=20px float=right caption=Atomistic input file generated using the SolutionRasterizer.

## Multiphysics Coupling
* Mechanics Coupling - Coupling phase field equations with mechanics

## Phase field sub-systems

### Nucleation
* [Discrete Nucleation](Nucleation/Discrete Nucleation.md) - Insertion of nuclei according to a nucleation probability density field
* [Langevin Noise](Nucleation/Langevin Noise.md) - Fluctuation based nucleation

### Grain Growth
* Grain Growth Model - Background on the phase field model implemented in MOOSE
* Grain Tracker Algorithm
* Grain Boundary Anisotropy
* Elastic Driving Force For Grain Growth

## Initial Conditions
* Initial Conditions - Basic phase field initial conditions
* Image Reader - Reconstructing initial conditions from images (SEM, optical, etc.)
* [EBSD Reader](ICs/EBSD.md) - Reconstructing initial conditions from EBSD and EDS data

# Tutorials
* Fe-Cr Phase Decomposition - Illustrates using parsed function kernels to create a two phase decomposition simulation

## Misc
* [Automatic Differentiation](Function Materials/Automatic Differentiation.md)
* [Function Materials](phase_field/FunctionMaterials.md)
* [Just in Time Compilation](Function Materials/JIT Compilation.md)
