# AddUELFunctions

!syntax description /Functions/AddUELFunctions/AddUELFunctions

## Description

The `AddUELFunctions` action reads the `*Amplitude` tables from an
[AbaqusUELMesh](mesh/AbaqusUELMesh.md) and adds one [PiecewiseLinear](PiecewiseLinear.md) function
for each table. The time column of the table becomes the `x` data and the amplitude column becomes
the `y` data of the function.

[AbaqusPredefAux](auxkernels/AbaqusPredefAux.md) uses these functions for `*Field` entries that
name an amplitude.

The action requires a mesh of type `AbaqusUELMesh`.

## Usage

Add the action under the `Functions` block:

!listing modules/solid_mechanics/test/tests/uel_mesh/amplitude_field.i block=Functions

!syntax parameters /Functions/AddUELFunctions/AddUELFunctions
