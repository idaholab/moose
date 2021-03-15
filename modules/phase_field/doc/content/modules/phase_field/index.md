# Phase Field Module

!media phase_field/pf_module_examples_vertical.png style=width:200px;padding-left:20px;float:right;
       caption=Phase field results produced using the MOOSE Phase Field Module

- [Systems](phase_field/systems.md)

The MOOSE phase field module is a library for simplifying the implementation of simulation tools that
employ the phase field model. Multiphysics capability that includes mechanics and heat conduction can
be obtained by employing the tensor mechanics and heat conduction modules. More information about
this module is found below:

## Basic Phase Field Model Information

- [Basic Phase Field Equations](phase_field/Phase_Field_Equations.md): Basic information about the equations underlying the phase field module
- [Expression Builder](FunctionMaterials/ExpressionBuilder.md): Using automatic differentiation of free energy material objects
- [Solving Phase Field Models](phase_field/Solving.md): Basic info about solving phase field models
- [Function Material Kernels](phase_field/FunctionMaterialKernels.md): Working with Function Materials that carry around their own derivatives
- [Function Materials](phase_field/FunctionMaterials.md): Creating material properties based on the value of an arbitrary function expression
- [Phase Field Model Units](phase_field/Phase_Field_Model_Units.md): Discussion of units in phase field models
- [Anisotropy](phase_field/Anisotropy.md): Support of anisotropic mobilities and interfacial energies
- [CALPHAD](phase_field/CALPHAD.md): Using thermodynamic databases to parameterize phase field models
- [Quantitative Two Component Polynomial Free Energies](phase_field/Quantitative.md): Simple two component models using polynomial free energies
- [FAQ](phase_field/FAQ.md): Frequently asked questions about the phase field modules

## Multiple Phase Models

MOOSE provides capabilities that enable the easy development of multiphase field model. A free energy expression has to be provided for each individual phase. Different systems exist to combine those _phase free energies_ into a _global free energy_.

- [Two-phase Models](MultiPhase/WBMTwoPhase.md): Two phases, one phase order parameter
- [Kim-Kim-Suzuki Model](MultiPhase/KKS.md): per-phase concentrations, two phases
- [Multiphase Models](MultiPhase/WBM.md): $N$ phases, $N$ phase order parameters
- Grand Potential Model: solving a Legendre transform of the phase field equations, where the independent variable is the chemical potential

!media media/phase_field/solutionrasterizer.png style=width:200px;padding-left:20px;float:right; caption=Atomistic input file generated using the SolutionRasterizer.

## Multiphysics Coupling

- [Mechanics Coupling](phase_field/Mechanics_Coupling.md) - Coupling phase field equations with mechanics

## Phase field sub-systems

### Nucleation

- [Discrete Nucleation](Nucleation/DiscreteNucleation.md): Insertion of nuclei according to a nucleation probability density field
- [Langevin Noise](Nucleation/LangevinNoise.md): Fluctuation based nucleation

### Grain Growth

- [Grain Growth Model](Grain_Growth_Model.md): Background on the phase field model implemented in MOOSE
- [Grain Tracker Algorithm](/GrainTracker.md)
- [Grain Boundary Anisotropy](Grain_Boundary_Anisotropy.md)
- [Elastic Driving Force For Grain Growth](Elastic_Driving_Force_Grain_Growth.md)

## Initial Conditions

- [Initial Conditions](Initial_Conditions.md): Basic phase field initial conditions
- [Polycrystal Initial Conditions](ICs/PolycrystalICs.md)
- Image Reader: Reconstructing initial conditions from images (SEM, optical, etc.)
- [EBSD Reader](ICs/EBSD.md): Reconstructing initial conditions from EBSD and EDS data

# Tutorials

- [Fe-Cr Phase Decomposition](Tutorial.md): Illustrates using parsed function kernels to create a two phase decomposition simulation

## Misc

- [Automatic Differentiation](FunctionMaterials/AutomaticDifferentiation.md)
- [Just in Time Compilation](FunctionMaterials/JITCompilation.md)
