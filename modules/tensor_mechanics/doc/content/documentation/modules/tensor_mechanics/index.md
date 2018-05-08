# Tensor Mechanics Module class=center style=font-weight:500;font-size:200%;

!media tensor_mechanics/lwr_3quarter_demo.png
       id=lws_three_quarter_model
       style=width:300px;padding-left:20px;float:right;
       caption=Thermo-mechanical stress analysis of a reactor pressure vessel.

The +Tensor Mechanics module+ is a library of simulation tools that solve
continuum mechanics problems. It provides a simple approach for implementing
even advanced mechanics models:

- Plug-n-play design enables users to encorporate the relevant physics for specific and varied simulations
- Tensor implementation matches mathematical theory
- Straight-forward procedure for adding new physics
- Complete [System Reference List](tensor_mechanics/systems.md) for the tensor mechanics code

The tensor mechanics system can be used to simulation both linear and finite strain mechanics, including

- Elasticity
- Plasticity and micromechanics plasticity
- Creep
- Damage due to cracking and property degradation

## Explore the Capabilities

The +Tensor Mechanics module+ is used in a variety of pure mechanics simulations
and in combined physics simulations with the Heat Transfer, Phase Field, Contact,
Porous Flow, and XFEM modules; use the MOOSE combined module to perform simulations
with multiple physics modules. The following figures show results from a few
different simulations performed by tensor mechanics module users.

!media media/bridge_demo.png
       id=example1
       style=width:28%;margin-right:2%;float:left;
       caption=Continuum mechanics analysis of an arch bridge with effective stress concentrations.

!media tensor_mechanics/3D_shear_failure_screen_shot.png
       id=coal_mining_example
       style=width:32%;margin-right:2%;float:left;
       caption=Evolution of rock failure zone in a 300m-wide, 400m-deep panel in a coal mining application.

!media tensor_mechanics/polyxtal27_temp.gif
       style=width:35%;float:left;
       id=demo_problem_bridge
       caption=Evolution of the resolved shear stress on the $\mathrm{{[}112{]}(11\bar{1})}$ slip system in a polycrystalline simulation of BCC Iron under tensile loading.


## Start Modeling

Interested in performing some of these simulations yourself? Use the links below
to learn more about the tensor mechanics module and to get started with your own
continuum mechanics and combined physics simulations.

!row!
!col! class=s12 m4 l4 icon=device_hub

### Plug-n-Play Structure Overview class=center style=font-weight:200;

Familiarize yourself with the [Plug-n-Play Structure](tensor_mechanics/plug_n_play.md)
used by tensor mechanics and then dive into the mathematical theory:

- [Strain Calculations](tensor_mechanics/Strains.md)
- [Stress Models](tensor_mechanics/Stresses.md)
- [Stress Divergence](tensor_mechanics/StressDivergence.md)
- [Tensor Definitions](tensor_mechanics/TensorClasses.md)

These classes make up the core of the tensor mechanics module.

!col-end!

!col! class=s12 m4 l4 icon=school

### Examples and Tutorials class=center style=font-weight:200;

Get started running your own tensor mechanics simulations by exploring the
introductory tutorials and examples. Next browse through the information:

- [Visualizing Tensors](tensor_mechanics/VisualizingTensors.md)
- [System Reference List](tensor_mechanics/systems.md)

Now you're ready to start creating your own mechanics simulations.

 <!-- the guide on
 to postprocess
your new results. After completing these, browse through
the complete  and start
creating your own mechanics simulations. -->

!col-end!

!col! class=s12 m4 l4 icon=storage

### Advanced Options and Features class=center style=font-weight:200;

Explore the different ways to use the tensor mechanics module:

- [Fracture Integrals](tensor_mechanics/FractureIntegrals.md)
- Smeared Cracking
- Crystal Plasticity
- Multiple Inelastic Stresses
- [C0 Timoshenko Beam](tensor_mechanics/C0TimoshenkoBeam.md)
- [Generalized Plane Strain](tensor_mechanics/generalized_plane_strain.md)

Review the helpful advice on using the
[Volumetric Locking Correction](tensor_mechanics/VolumetricLocking.md) and about
setting appropriate [Convergence Criteria](tensor_mechanics/Convergence.md).
!col-end!
!row-end!

The tensor mechanics module is being developed by users at national laboratories
and universities around the world. Contact the developers through the
[moose-users email list](help/contact_us.md).

## Developing New Tensor Mechanics Code

Consider becoming a developer yourself.
The tensor mechanics module uses code syntax based on tensor forms. This approach
allows the constitutive tensor equations to be implemented, clearly and concisely,
in the same format as written in mathematical notation.
Follow the MOOSE standards for [contributing code and documentation](utilities/MooseDocs/generate.md).
