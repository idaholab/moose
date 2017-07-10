# Porous Flow

The PorousFlow module is a library of physics for fluid and heat flow in porous
media. It is formulated in an extremely general manner, so is capable of solving
problems with an arbitrary number of phases and fluid components.

By simply adding pieces of physics together in an input file, the PorousFlow
module enables the user to model problems with any combination of fluid, heat
and geomechanics.

## Theoretical foundation

The equations governing motion of fluid and heat in porous media that are implemented
in `Kernels` in the PorousFlow module.

- [Governing equations](/porous_flow/governing_equations.md)

## Available models

Several different flow models are available in PorousFlow.

General formulations for the following cases are possible:

- [Single phase](/porous_flow/singlephase.md)
- [Multiphase](/porous_flow/multiphase.md)

Specialised formulations for miscible two-phase flow are also provided, that use
a [persistent](/porous_flow/persistent_variables.md) set of primary variables and a [compositional flash](/porous_flow/compositional_flash.md) to calculate the partitioning
of fluid components amongst fluid phases:

- [Water and non-condensable gas](/porous_flow/waterncg.md)
- [Brine and CO$_2$](/porous_flow/brineco2.md)

## Material laws

Material laws implemented in PorousFlow.

- [Capillary pressure](/porous_flow/capillary_pressure.md)
- [Relative permeability](/porous_flow/relative_permeability.md)
- [Permeability](/porous_flow/permeability.md)
- [Porosity](/porous_flow/porosity.md)
- [Diffusivity](/porous_flow/diffusivity.md)

## Fluid equation of states

PorousFlow uses formulations contained in the [Fluid
Properties](/fluid_properties/index.md) module to calculate fluid properties
such as density or viscosity.

 - [Using fluid properties](/porous_flow/fluids.md)
 - [Available fluids](/fluid_properties/index.md)

## Boundary conditions

Several boundary conditions useful for many simulations are provided.

 - [Boundaries](/porous_flow/boundaries.md)

## Sources/sinks

A number of fluid and/or heat sources/sinks are available for use in PorousFlow.

 - [Sources/sinks](/porous_flow/sinks.md)

## Implementation details

Details about numerical issues.

- [Mass lumping](/porous_flow/mass_lumping.md)
- [Upwinding](/porous_flow/upwinding.md)
- [Preconditioning and solvers](/porous_flow/solvers.md)
- [Convergence criteria](/porous_flow/convergence.md)

## The Dictator

The [`PorousFlowDictator`](/porous_flow/PorousFlowDictator.md) is a `UserObject`
that holds information about the nonlinear variables used in the PorousFlow module,
as well as the number of fluid phases and fluid components in each simulation.

Other PorousFlow objects, such as `Kernels` or `Materials` query the `PorousFlowDictator`
to make sure that only valid fluid components or phases are used.

!!! note:
    A `PorousFlowDictator` **must** be present in all simulations!

## Additional MOOSE objects

The PorousFlow module also includes a number of additional MOOSE objects to aid
users in extracting calculated values for visualising results.

### AuxKernels

The following `AuxKernels` can be used to save properties and data to `AuxVariables`,
which can then be used as input for other MOOSE objects, or saved to output files and
used to visualise results.

- [`PorousFlowDarcyVelocityComponent`](/porous_flow/PorousFlowDarcyVelocityComponent.md):
Calculates the Darcy velocity of the fluid
- [`PorousFlowPropertyAux`](/porous_flow/PorousFlowPropertyAux.md): Extracts properties
from the model.

### Postprocessors

A number of `Postprocessors` are available:

- [`PorousFlowFluidMass`](/porous_flow/PorousFlowFluidMass.md): Calculates the mass of a
fluid component $\kappa$
- [`PorousFlowHeatEnergy`](/porous_flow/PorousFlowHeatEnergy.md): Calculates the heat energy
