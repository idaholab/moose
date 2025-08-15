# AbaqusUELStateVariables

!syntax description /VectorPostprocessors/AbaqusUELStateVariables

## Description

`AbaqusUELStateVariables` exports state variables managed by an
[AbaqusUELMeshUserElement](userobjects/AbaqusUELMeshUserElement.md) to vector postprocessor
outputs for inspection and CSV writing.

- `uel`: the name of the `AbaqusUELMeshUserElement` providing state variables and active element set.
- Produces an element ID vector `abaqus_elem_id` and one or more value vectors. By default, value
  vectors are named `state_var_1`, `state_var_2`, ... up to the number of state variables.
- `column_names`: optional custom names to apply to the output value vectors (in order).
- `split`: splits the state variables into this many equal batches, outputting multiple rows per
  element. When `split > 1`, an auxiliary integer vector `IntP` stores the 1-based batch index.

State variables are pulled for the active elements handled by the referenced UEL user object and are
thread-safe merged.

### Usage

Example configuration from a test input:

!listing modules/solid_mechanics/test/tests/uel_mesh/cube_uel_bcs.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/AbaqusUELStateVariables

!syntax inputs /VectorPostprocessors/AbaqusUELStateVariables

!syntax children /VectorPostprocessors/AbaqusUELStateVariables

