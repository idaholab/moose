# Solid Mechanics Module

The Solid Mechanics module is a library of simulation tools that solve
continuum mechanics problems. It provides a simple approach for implementing
even advanced mechanics models.

The +Solid Mechanics module+ is used in a variety of pure mechanics simulations
and in combined physics simulations with the Heat Transfer, Phase Field, Contact,
Porous Flow, and XFEM modules. The following figures show results from a few
different simulations performed by solid mechanics module users.

!row!

!media solid_mechanics/3D_shear_failure_screen_shot.png
       style=width:34%;float:right;padding-top:2.5%;
       caption=Evolution of rock failure zone in a 300m-wide, 400m-deep panel in a coal mining application.

!media solid_mechanics/polyxtal27_temp.gif
       style=width:32%;float:right;margin-left:1%;margin-right:1%;
       caption=Evolution of the resolved shear stress on the $\mathrm{{[}112{]}(11\bar{1})}$ slip system in a polycrystalline simulation of BCC Iron.

!media solid_mechanics/lwr_3quarter_demo.png
       style=width:32%;float:right;
       caption=Thermo-mechanical stress analysis of a reactor pressure vessel.

!row-end!


The Solid Mechanics module follows strict software quality guidelines, refer to
[Solid Mechanics SQA](solid_mechanics/sqa/index.md) for more information.

Interested in performing some of these simulations yourself? Use the links below
to learn more about the solid mechanics module and to get started with your own
continuum mechanics and combined physics simulations.

## Kinematics and Governing Equations

Set up a mechanics problem with the QuasiStatic Physics, and learn the underlying theory:

- [SolidMechanics/QuasiStatic Physics](/Physics/SolidMechanics/QuasiStatic/index.md)
- [Kinematics](solid_mechanics/Kinematics.md)
- [Kinematic Approximations](solid_mechanics/KinematicApproximations.md)
- [Stabilization](solid_mechanics/Stabilization.md)
- [Balance of Linear Momentum](solid_mechanics/BalanceOfLinearMomentum.md)

  - [Total Lagrangian](TotalLagrangianStressDivergence.md)
  - [Updated Lagrangian](UpdatedLagrangianStressDivergence.md)

- [Dynamics](solid_mechanics/Dynamics.md)

## Constitutive Models

Select from the available stress models or add your own:

- [Objective Stress Rates](solid_mechanics/ObjectiveStressRates.md)
- [Custom Constitutive Models](solid_mechanics/CustomConstitutiveModels.md)
- [NEML2 Constitutive Model Library](solid_mechanics/NEML2Models.md)
- Legacy implementations

  - [Smeared Cracking](/ComputeSmearedCrackingStress.md)
  - [Multi-Surface Creep/Plasticity](/ComputeMultipleInelasticStress.md)
  - [Crystal Plasticity](/ComputeMultipleCrystalPlasticityStress.md)
  - [Viscoplasticity](/ADViscoplasticityStressUpdate.md)
  - [LAROMANCE](/LAROMANCE.md)

## Advanced Features

Specialized capabilities for particular problem classes:

- [Generalized Midpoint Rule](solid_mechanics/GeneralizedMidpointRule.md)
- [Homogenization](solid_mechanics/Homogenization.md)
- Special Elements

  - [C0 Timoshenko Beam](solid_mechanics/C0TimoshenkoBeam.md)
  - [Shell Elements](solid_mechanics/ShellElements.md)
  - [Cohesive Elements](CohesiveZone/index.md)

- [Fracture Integrals](solid_mechanics/FractureIntegrals.md)
- [Frequency Domain Dynamics](/1d_elastic_waves.md)
- [Isogeometric Analysis](solid_mechanics/examples/cframe_iga.md)

## Examples and Tutorials

Get started with introductory tutorials and examples:

- [Introduction](solid_mechanics/tutorials/introduction/index.md)
- [Outputting Tensor Components](solid_mechanics/VisualizingTensors.md)
- [Setting Convergence Criteria](solid_mechanics/Convergence.md)
- [Module Documentation List](solid_mechanics/systems.md)
