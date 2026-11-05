# AbaqusForceBC

!syntax description /NodalKernels/AbaqusForceBC

## Description

`AbaqusForceBC` complements [AbaqusEssentialBC](AbaqusEssentialBC.md) in realizing Abaqus
`*Boundary` behavior. When a Dirichlet constraint is deactivated at the beginning of a step, the
corresponding nodal reaction force from the prior state is applied as an external nodal force and
then ramped to zero over the course of the step using the step fraction `d in [0,1]`:

- Reads the concentrated reaction force at the beginning of the step via
  [AbaqusUELStepUserObject](userobjects/AbaqusUELStepUserObject.md).
- Applies `(1 - d) * begin_force` at the node for the current Abaqus variable ID.
- Operates on the synthetic `abaqus_bc_union_boundary` created by
  [AbaqusUELMesh](AbaqusUELMesh.md).

This is a `NodalKernel` because it adds a force contribution rather than prescribing a value.

To enable capturing reaction forces from the UEL bulk contributions, ensure the problem declares the
`AbaqusUELTag` vector tag (used by [AbaqusUELMeshUserElement](AbaqusUELMeshUserElement.md)).

## Usage

Example configuration (paired with `AbaqusEssentialBC`):

!listing modules/solid_mechanics/test/tests/uel_mesh/elastic.i block=NodalKernels

!syntax parameters /NodalKernels/AbaqusForceBC

!syntax inputs /NodalKernels/AbaqusForceBC

!syntax children /NodalKernels/AbaqusForceBC
