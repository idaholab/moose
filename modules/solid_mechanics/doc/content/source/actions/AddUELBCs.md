# AddUELBCs

## Description

The `AddUELBCs` action configures Abaqus-style boundary conditions from an
[AbaqusUELMesh](mesh/AbaqusUELMesh.md) with user elements. It wires up the step coordination and the
two boundary-condition components needed to reproduce Abaqus `*Boundary` behavior:

- Creates an [AbaqusUELStepUserObject](userobjects/AbaqusUELStepUserObject.md) (named
  `abaqus_step_uo`) that interprets the Abaqus steps and exposes begin/end values, begin solution,
  and begin reaction forces per Abaqus variable ID.
- Adds one [AbaqusEssentialBC](bcs/AbaqusEssentialBC.md) per Abaqus variable to prescribe
  Dirichlet constraints on the synthetic `abaqus_bc_union_boundary` created by the UEL mesh, with
  ramping according to the step fraction.
- Adds one [AbaqusForceBC](nodalkernels/AbaqusForceBC.md) per Abaqus variable to apply prior
  nodal reaction forces when an essential BC is deactivated, ramping them to zero during the step.
- Requires a mesh of type `AbaqusUELMesh` and that the problem declares the `AbaqusUELTag` vector
  tag (so reaction forces from UEL residuals can be captured).
- Assumes the solver variables matching Abaqus DOF IDs already exist (e.g., via
  [AddUELVariables](AddUELVariables.md)).

### Usage

Use the `BCs/Abaqus` action block to auto-add the step user object, essential BCs, and force BCs:

!listing modules/solid_mechanics/test/tests/uel_mesh/elastic_action.i block=BCs

