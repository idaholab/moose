# Tensor Mechanics Module

- [System Documentation List](tensor_mechanics/systems.md)

The Tensor Mechanics module is a library of simulation tools that solve
continuum mechanics problems. It provides a simple approach for implementing
even advanced mechanics models:

- Plug-n-play design enables users to incorporate the relevant physics for specific and varied simulations
- Tensor implementation matches mathematical theory
- Straight-forward procedure for adding new physics

The tensor mechanics system can be used to simulate both linear and finite strain mechanics, including
Elasticity and Cosserat elasticity, Plasticity and micromechanics plasticity, Creep, and
Damage due to cracking and property degradation

## Explore the Capabilities and Start Modeling

The +Tensor Mechanics module+ is used in a variety of pure mechanics simulations
and in combined physics simulations with the Heat Transfer, Phase Field, Contact,
Porous Flow, and XFEM modules; use the MOOSE combined module to perform simulations
with multiple physics modules. The following figures show results from a few
different simulations performed by tensor mechanics module users.

!row!

!media tensor_mechanics/3D_shear_failure_screen_shot.png
       style=width:34%;float:right;padding-top:2.5%;
       caption=Evolution of rock failure zone in a 300m-wide, 400m-deep panel in a coal mining application.

!media tensor_mechanics/polyxtal27_temp.gif
       style=width:32%;float:right;margin-left:1%;margin-right:1%;
       caption=Evolution of the resolved shear stress on the $\mathrm{{[}112{]}(11\bar{1})}$ slip system in a polycrystalline simulation of BCC Iron.

!media tensor_mechanics/lwr_3quarter_demo.png
       style=width:32%;float:right;
       caption=Thermo-mechanical stress analysis of a reactor pressure vessel.

!row-end!


Interested in performing some of these simulations yourself? Use the links below
to learn more about the tensor mechanics module and to get started with your own
continuum mechanics and combined physics simulations.

!row!
!col! small=12 medium=4 large=4 icon=device_hub

### Plug-n-Play Structure Overview class=center style=font-weight:200;

Familiarize yourself with the [Plug-n-Play Structure](tensor_mechanics/plug_n_play.md)
used by tensor mechanics and then dive into the mathematical theory:

- [Strain Calculations](tensor_mechanics/Strains.md)
- [Stress Models](tensor_mechanics/Stresses.md)
- [Stress Divergence](tensor_mechanics/StressDivergence.md)
- [Tensor Definitions](tensor_mechanics/TensorClasses.md)

These types of code classes make up the core of the tensor mechanics module.

!col-end!

!col! small=12 medium=4 large=4 icon=school

### Examples and Tutorials class=center style=font-weight:200;

Get started running your own tensor mechanics simulations by exploring the
introductory tutorials and examples. Next browse through the information:

- [Introduction](tensor_mechanics/tutorials/introduction/index.md)
- [Visualizing Tensors](tensor_mechanics/VisualizingTensors.md)
- [Setting Convergence Criteria](tensor_mechanics/Convergence.md)
- [Module Documentation List](tensor_mechanics/systems.md)

Now you're ready to start creating your own mechanics simulations.

!col-end!

!col! small=12 medium=4 large=4 icon=storage

### Advanced Features class=center style=font-weight:200;

Explore the different ways to use the tensor mechanics module by browsing the
introductory theory pages on the various models:

- [Volumetric Locking Correction](tensor_mechanics/VolumetricLocking.md)
- [Smeared Cracking](/ComputeSmearedCrackingStress.md)
- [Multiple Inelastic Stresses](/ComputeMultipleInelasticStress.md)
- [Generalized Plane Strain](tensor_mechanics/generalized_plane_strain.md)
- [Fracture Integrals](tensor_mechanics/FractureIntegrals.md)
- [Crystal Plasticity](/ComputeMultipleCrystalPlasticityStress.md)
- [C0 Timoshenko Beam](tensor_mechanics/C0TimoshenkoBeam.md)
- [Dynamics](tensor_mechanics/Dynamics.md)
- [Viscoplasticity](/ADViscoplasticityStressUpdate.md)
- [Cohesive Zone Modeling](CohesiveZoneMaster/index.md)
- [Shell elements](tensor_mechanics/ShellElements.md)
- [Reduced Order Models](/LAROMANCE.md)
- [Frequency Domain Dynamics](/1d_elastic_waves.md)
- [Isogeometric Analysis](tensor_mechanics/examples/cframe_iga.md)
!col-end!
!row-end!

## New Mechanics Kernels

The Tensor Mechanics module currently has two, partially interoperable
underlying systems:

- The current system based on the [StressDivergenceTensors](/StressDivergenceTensors.md) and related kernels.
- A newer system based on the [TotalLagrangianStressDivergence](/TotalLagrangianStressDivergence.md) and [UpdatedLagrangianStressDivergence](/UpdatedLagrangianStressDivergence.md) kernels.

The current system may suffer from convergence issues caused by non-exact Jacobians for large deformations problems when not used with the Automatic Differentiation variants of the kernels and underlying materials.
The newer system (referred to in the documentation as the *Lagrangian* kernels) has exact Jacobians and also includes:

- A [common interface](tensor_mechanics/LagrangianKernelTheory.md) for running small or large deformation problems that simplifies how input files are setup and makes it easier to switch between different kinematic and material models.
- An [improved material system](tensor_mechanics/NewMaterialSystem.md), that provides multiple options for implementing new materials models.  The new material system can also automatically convert a small deformation material model to large deformation kinematics by integrating a user-select objective stress rate.
- Both [total Lagrangian](/TotalLagrangianStressDivergence.md) and [updated Lagrangian](/UpdatedLagrangianStressDivergence.md) formulations.
- A [homogenization system](/tensor_mechanics/Homogenization.md) designed to enforce cell-average deformation or stress conditions over a periodic unit cell.
- [Stabilization for linear elements](/tensor_mechanics/Stabilization.md) for use in incompressible or nearly-incompressible problems through a $\bar{\boldsymbol{F}}$ formulation.

The newer kernels are compatible with the existing MOOSE materials via the [ComputeLagrangianWrappedStress](/ComputeLagrangianWrappedStress.md) object.  This object maps the output from the existing MOOSE material system into the format expected
by the Lagrangian kernels.

The Lagrangian kernels are feature-complete with the original kernels -- all simulations using the original kernels should be able to be converted to use the
Lagrangian kernels.  However, users should be aware that modules that couple to `"stress"` for the Cauchy stress using the original kernels should now either couple to `"cauchy_stress"` or `"pk1_stress"`, as appropriate, in the Lagrangian kernels.

Users should consider using the new system
for problems where numerical convergence is critical -- for example problems with large material or geometric nonlinearities -- or
for problems where the stress update provided by the constitutive model is very expensive, as the new kernels will achieve convergence
in many fewer nonlinear iterations, when compared to the older system.

Both the new and old systems are accessible through the [TensorMechanics/MasterAction](/Modules/TensorMechanics/Master/index.md), which simplifies the process of
setting up and running models.

## Developing New Tensor Mechanics Code

The TensorMechanics module is being developed by users at national laboratories
and universities around the world. Learn how to get in touch with the developers
on the [help/contact_us.md optional=True] page.

Consider becoming a developer yourself.
The tensor mechanics module uses code syntax based on tensor forms. This approach
allows the constitutive tensor equations to be implemented, clearly and concisely,
in the same format as written in mathematical notation.
Follow the MOOSE standards for [contributing](framework/contributing.md).

## Software Quality

The Tensor Mechanics module follows strict software quality guidelines, refer to
[Tensor Mechanics SQA](tensor_mechanics/sqa/index.md) for more information.
