# AddUELICs

## Description

The `AddUELICs` action reads field initial conditions from the [AbaqusUELMesh](mesh/AbaqusUELMesh.md)
and creates corresponding MOOSE `ConstantIC` objects on automatically generated node-set boundaries.

- For each initial condition entry in the Abaqus input, this action:
  - Derives the variable name from the Abaqus variable ID (via `AbaqusUELMesh::getVarName`).
  - Creates a boundary named `abaqus_<nodeset>_<var_name>` and populates it with the nodes from the
    Abaqus node set (only where the variable exists).
  - Registers the new boundary with the UEL mesh and adds a `ConstantIC` with the prescribed value.
- Requires that the mesh is of type `AbaqusUELMesh`.

This action parallels the `AddUELBCs` action but targets initial conditions instead of boundary
conditions.

### Usage

Place under the `ICs` section to populate initial conditions defined in the Abaqus file:

```
[ICs]
  [AddUELICs]
  []
[]
```
