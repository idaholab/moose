# AddUELVariables

## Description

The `AddUELVariables` action inspects the [AbaqusUELMesh](mesh/AbaqusUELMesh.md) to determine all
degrees of freedom required by the UEL definitions present in the Abaqus input and then adds the
corresponding MOOSE variables automatically.

- Variables are created as FIRST-order Lagrange fields, one per Abaqus DOF ID, with consistent
  names produced by `AbaqusUELMesh::getVarName` (e.g., 1–3 -> `disp_x/y/z`, 4–6 -> `rot_x/y/z`).
- Each variable is block-restricted to the subdomains (bit masks) in which that DOF exists.
- Requires that the mesh is of type `AbaqusUELMesh`.

### Usage

Add under the `Variables` section to auto-populate the solver variables from the Abaqus UEL input:

!listing modules/solid_mechanics/test/tests/uel_mesh/cube_uel_bcs.i block=Variables
