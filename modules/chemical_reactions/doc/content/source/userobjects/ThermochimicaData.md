# ThermochimicaData

!syntax description /UserObjects/ThermochimicaData

## Description

`ThermochimicaData` performs the Thermochimica equilibrium calculations configured by the
[ChemicalComposition action](ChemicalCompositionAction.md). It evaluates the configured
temperature, pressure, and element compositions at each selected node or element and writes the
requested equilibrium quantities to the auxiliary variables created by the action.

Available quantities include amounts and fractions, element and thermodynamic-component
potentials, vapor pressures, phase Gibbs energies and driving forces, and the integral system Gibbs
energy. See [ChemicalCompositionAction.md#thermochemical-outputs] for their units and selection
syntax.

This object is created internally by `ChemicalComposition` and is not intended to be added directly
to the `[UserObjects]` block. Configure its evaluation location, outputs, batching, warm-start
strategy, block restriction, and execution schedule through the action parameters.

!syntax inputs /UserObjects/ThermochimicaData

!syntax children /UserObjects/ThermochimicaData
