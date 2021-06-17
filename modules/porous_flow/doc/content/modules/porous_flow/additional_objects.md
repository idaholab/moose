# Additional MOOSE objects

The PorousFlow module also includes a number of additional MOOSE objects to aid
users, whether to assist in extracting calculated values for visualizing results,
implement advanced capabilities, or to simplify the input files.

## Actions

The `Actions` system allows programatic addition of various input file objects, significantly
simplifying the input files for various simulations.

- [PorousFlowBasicTHM](PorousFlowBasicTHM.md): Fully-saturated, single-phase, single-component fluid flow
- [PorousFlowFullySaturated](PorousFlowFullySaturated.md): Fully-saturated, multi-component, single-phase fluid flow
- [PorousFlowUnsaturated](PorousFlowUnsaturated.md): Saturated-unsaturated, multi-component, single-phase fluid flow

## AuxKernels

The following `AuxKernels` can be used to save properties and data to `AuxVariables`,
which can then be used as input for other MOOSE objects, or saved to output files and
used to visualise results.

- [PorousFlowDarcyVelocityComponent](PorousFlowDarcyVelocityComponent.md):
  Calculates the Darcy velocity of the fluid
- [PorousFlowDarcyVelocityComponentLowerDimensional](PorousFlowDarcyVelocityComponentLowerDimensional.md):
  Calculates the Darcy velocity of the fluid in a lower-dimensional element
- [PorousFlowPropertyAux](PorousFlowPropertyAux.md):
  Extracts material properties from the model

## Initial conditions

Some models that use persistent primary variables also include initial condition objects to calculate
the initial value of these persistent primary variables for a given set of porepressure and temperature.

- [PorousFlowFluidStateIC](PorousFlowFluidStateIC.md): Calculates total mass fraction of component

## Postprocessors

A number of `Postprocessors` are available:

- [PorousFlowFluidMass](PorousFlowFluidMass.md): Calculates the mass
  of a fluid component $\kappa$
- [PorousFlowHeatEnergy](PorousFlowHeatEnergy.md): Calculates the heat energy
- [PorousFlowPlotQuantity](PorousFlowPlotQuantity.md): Records the flow into/out of a sink during time step

## UserObjects

The following `UserObjects` are provided:

- [PorousFlowDictator](PorousFlowDictator.md): Provides information to all other objects
- [PorousFlowSumQuantity](PorousFlowSumQuantity.md): Records the total flow into/out of a sink
- [PorousFlowAdvectiveCalculator](kt.md): Used in [Kuzmin-Turek](kt.md) stabilization
