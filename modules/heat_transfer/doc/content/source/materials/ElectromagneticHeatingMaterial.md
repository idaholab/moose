# ElectromagneticHeatingMaterial

!syntax description /Materials/ElectromagneticHeatingMaterial

## Overview

`ElectromagneticHeatingMaterial` provides the electric field and residuals of electromagnetic/electrostatic
heating based objects as material properties. In particular, `ElectromagneticHeatingMaterial` provides the
residuals for [ADJouleHeatingSource](ADJouleHeatingSource.md) and [JouleHeatingHeatGeneratedAux](JouleHeatingHeatGeneratedAux.md) within the Heat Transfer module.

For the formulation of the electric field, users can declare:

- ELECTROMAGNETIC: where the electric field can be supplied by the electromagnetic module
- ELECTROSTATIC: where the user provides an electrostatic potential and the electric field ($E$) is defined as $E = \nabla V$,  where $V$ is the electrostatic potential

For the formulation of the residuals, user can declare:

- TIME DOMAIN: where the electric field is assumed to be derived from the time dependent Maxwell's equations
- FREQUENCY DOMAIN: where the electric field is assumed to be derived from the frequency dependent Maxwell's equations. When using the frequency domain, the electric field is assumed to have a real and complex component.

## Example Input File Syntax

An electrostatic example of how to use `ElectromagneticHeatingMaterial` can be found in the
heat transfer module test `transient_ad_jouleheating.i`.

!listing modules/heat_transfer/test/tests/joule_heating/transient_ad_jouleheating.i block=Materials/ElectromagneticMaterial

An electromagnetic example of how to use `ElectromagneticHeatingMaterial` can be found in the
combined module test `aux_microwave_heating.i`.

!listing modules/combined/test/tests/electromagnetic_joule_heating/aux_microwave_heating.i block=Materials/ElectromagneticMaterial

!syntax parameters /Materials/ElectromagneticHeatingMaterial

!syntax inputs /Materials/ElectromagneticHeatingMaterial

!syntax children /Materials/ElectromagneticHeatingMaterial
