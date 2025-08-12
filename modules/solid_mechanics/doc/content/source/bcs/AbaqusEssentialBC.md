# AbaqusEssentialBC

!syntax description /BCs/AbaqusEssentialBC

## Description

`AbaqusEssentialBC` applies Dirichlet (essential) boundary conditions that are defined in an Abaqus
input file and loaded through an [AbaqusUELMesh](mesh/AbaqusUELMesh.md). It consumes step and
boundary-condition data provided by [AbaqusUELStepUserObject](userobjects/AbaqusUELStepUserObject.md)
and prescribes nodal values for a specific Abaqus variable ID across time steps.

- It operates on the synthetic boundary `abaqus_bc_union_boundary`, which contains the union of all
  nodes that appear in any Abaqus `*Boundary` definition. This is automatically created by
  `AbaqusUELMesh`.
- For a node that is newly activated in the current step, the value is ramped from the current
  solution at the beginning of the step to the step’s target value using the step fraction `d in [0,1]`:
  `value = d * end_value + (1 - d) * begin_solution`.
- For a node that remains constrained across the step, the value is ramped between the step’s
  begin and end values: `value = d * end_value + (1 - d) * begin_value`.
- If a node has no constraint for the current variable in the current step, this BC does not apply
  at that node.

`AbaqusEssentialBC` is typically used together with
[AbaqusForceBC](nodalkernels/AbaqusForceBC.md) to carry over nodal reaction forces when an
essential boundary condition is deactivated.

## Usage

The object is normally added by the `BCs/Abaqus` action. It can also be configured explicitly as
shown below:

!listing modules/solid_mechanics/test/tests/uel_mesh/elastic.i block=BCs

!syntax parameters /BCs/AbaqusEssentialBC

!syntax inputs /BCs/AbaqusEssentialBC

!syntax children /BCs/AbaqusEssentialBC

