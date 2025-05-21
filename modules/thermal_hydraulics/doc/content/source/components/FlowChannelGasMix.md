# FlowChannelGasMix

This component implements the [gas mixture flow model](thermal_hydraulics/theory_manual/gas_mix_model/index.md)
on a [flow channel](component_groups/flow_channel.md),
as [FlowChannel1Phase.md] does for the
[single-phase flow model](thermal_hydraulics/theory_manual/vace_model/index.md).

## Usage

The usage for this component is the same as for [FlowChannel1Phase.md] but
with the addition of the initial condition parameter [!param](/Components/FlowChannelGasMix/initial_mass_fraction)
for the mass fraction of the secondary gas.

!syntax parameters /Components/FlowChannelGasMix

## Mesh id=mesh

Mesh-related parameters of this component match [FlowChannel1Phase.md].

## Variables

Compared to [FlowChannel1Phase.md], this component adds the following solution variable:

| Variable | Symbol | Description |
| :- | :- | :- |
| `xirhoA` | $\xi \rho A$ | Secondary gas mass per unit length \[kg/m\] |

and the following auxiliary variable:

| Variable | Symbol | Description |
| :- | :- | :- |
| `mass_fraction` | $\xi$ | Mass fraction of the secondary gas |

## Material Properties

Compared to [FlowChannel1Phase.md], this component adds the material property:

| Material Property | Symbol | Description |
| :- | :- | :- |
| `mass_fraction` | $\xi$ | Mass fraction of the secondary gas |

!syntax inputs /Components/FlowChannelGasMix

!syntax children /Components/FlowChannelGasMix

!bibtex bibliography
