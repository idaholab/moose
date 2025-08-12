# AbaqusPredefAux

!syntax description /AuxKernels/AbaqusPredefAux

## Description

`AbaqusPredefAux` exposes Abaqus field initial conditions (PREDEF) from an
[AbaqusUELMesh](mesh/AbaqusUELMesh.md) as nodal AuxVariables. It reads the `*Initial Condition,
variable=<N>` data for a given Abaqus field number and assigns the corresponding value to nodes in
the referenced Abaqus node sets.

- Parameter `field` selects the Abaqus field number (`variable=` in the Abaqus `*Initial Condition`).
- Requires a nodal AuxVariable target; errors if the variable is not nodal.
- Updates on `INITIAL` by default (and on step changes) so values are available to components such as
  [AbaqusUELMeshUserElement](userobjects/AbaqusUELMeshUserElement.md) via its `external_fields`
  option.
- Only nodes present in the field’s node sets receive values; all others get 0.0.

## Usage

Declare AuxVariables for the fields you want to expose and populate them with `AbaqusPredefAux`:

!listing modules/solid_mechanics/test/tests/uel_mesh/cube_uel_bcs.i block=AuxKernels

!syntax parameters /AuxKernels/AbaqusPredefAux

!syntax inputs /AuxKernels/AbaqusPredefAux

!syntax children /AuxKernels/AbaqusPredefAux
