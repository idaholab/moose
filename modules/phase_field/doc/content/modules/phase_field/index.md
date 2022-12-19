# Phase Field Module

!media phase_field/pf_module_examples_vertical.png style=width:200px;padding-left:20px;float:right;
       caption=Phase field results produced using the MOOSE Phase Field Module

- [Systems](phase_field/systems.md)

The MOOSE phase field module is a library for simplifying the implementation of simulation tools that
employ the phase field model. Multiphysics capability that includes mechanics and heat conduction can
be obtained by employing the tensor mechanics and heat conduction modules. More information about
this module is found below:

## Basic Phase Field Model Information

- [phase_field/Phase_Field_Equations.md]: Basic information about the equations underlying the phase field module
- [phase_field/FunctionMaterials/ExpressionBuilder.md]: Using automatic differentiation of free energy material objects
- [phase_field/Solving.md]: Basic info about solving phase field models
- [phase_field/FunctionMaterialKernels.md]: Working with Function Materials that carry around their own derivatives
- [phase_field/FunctionMaterials.md]: Creating material properties based on the value of an arbitrary function expression
- [phase_field/Phase_Field_Model_Units.md]: Discussion of units in phase field models
- [phase_field/Anisotropy.md]: Support of anisotropic mobilities and interfacial energies
- [phase_field/CALPHAD.md]: Using thermodynamic databases to parameterize phase field models
- [phase_field/Quantitative.md]: Simple two component models using polynomial free energies
- [phase_field/FAQ.md]: Frequently asked questions about the phase field modules

## Multiple Phase Models

MOOSE provides capabilities that enable the easy development of multiphase field model. A free energy expression has to be provided for each individual phase. Different systems exist to combine those _phase free energies_ into a _global free energy_.

- [phase_field/MultiPhase/WBMTwoPhase.md]: Two phases, one phase order parameter
- [phase_field/MultiPhase/KKS.md]: per-phase concentrations, two phases
- [phase_field/MultiPhase/SLKKS.md]: per-phase concentrations, per-sublattice concentrations, multiple phases
- [phase_field/MultiPhase/WBM.md]: $N$ phases, $N$ phase order parameters
- [phase_field/MultiPhase/GrandPotentialMultiphase.md]: solving a Legendre transform of the phase field equations, where the independent variable is the chemical potential

!media media/phase_field/solutionrasterizer.png style=width:200px;padding-left:20px;float:right; caption=Atomistic input file generated using the SolutionRasterizer.

## Multiphysics Coupling

- [phase_field/Mechanics_Coupling.md]: Coupling phase field equations with mechanics

## Phase field sub-systems

### Nucleation

- [phase_field/Nucleation/DiscreteNucleation.md]: Insertion of nuclei according to a nucleation probability density field
- [phase_field/Nucleation/LangevinNoise.md]: Fluctuation based nucleation

### Grain Growth

- [phase_field/Grain_Growth_Model.md]: Background on the phase field model implemented in MOOSE
- [Grain Tracker Algorithm](/GrainTracker.md): Tracking arbitrary features on an unstructured mesh over time
- [phase_field/Grain_Boundary_Anisotropy.md]: For systems with misorientation dependence of GB properties
- [phase_field/Elastic_Driving_Force_Grain_Growth.md]: Coupling mechanics to influence grain growth

## Initial Conditions

- [phase_field/Initial_Conditions.md]: Basic phase field initial conditions
- [phase_field/ICs/PolycrystalICs.md]: Creating polycrystalline structures using a variety of methods
- [Image Reader](/ImageFunction.md): Reconstructing initial conditions from images (SEM, optical, etc.)
- [EBSD Reader](phase_field/ICs/EBSD.md): Reconstructing initial conditions from EBSD and EDS data

# Tutorials

- [Fe-Cr Phase Decomposition](phase_field/Tutorial.md): Illustrates using parsed function kernels to create a two phase decomposition simulation

## Misc

- [phase_field/FunctionMaterials/AutomaticDifferentiation.md]: Construct derivatives of a given expression in a symbolic way
- [phase_field/FunctionMaterials/JITCompilation.md]: Compilation of FunctionParser expressions
