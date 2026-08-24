# AbaqusUELStepUserObject

!syntax description /UserObjects/AbaqusUELStepUserObject

## Description

`AbaqusUELStepUserObject` processes the `*Step` information from an Abaqus input file loaded through
an [AbaqusUELMesh](AbaqusUELMesh.md). It builds a timeline of step durations and exposes the
current step index and fractional progress along the step. It also provides lookups for boundary
conditions and nodal forces that enable Abaqus-style activation/deactivation of nodal constraints.

- Executes on `INITIAL` and `TIMESTEP_BEGIN` to determine the current step based on simulation time.
- Provides `getBeginValues`/`getEndValues` maps for Abaqus variable IDs to nodal values, allowing
  boundary conditions to ramp between begin/end values during a step.
- Provides `getBeginSolution` for nodes and variables that become newly constrained during the step
  (ramping from the solution at the step start).
- Provides `getBeginForces` for nodes and variables that become unconstrained during the step,
  exposing the concentrated reaction force captured from the bulk UEL contributions via the
  `AbaqusUELTag` vector tag.

This user object is typically consumed by [AbaqusEssentialBC](AbaqusEssentialBC.md) and
[AbaqusForceBC](AbaqusForceBC.md) and is automatically set up by the
[BCs/Abaqus](BCs/Abaqus/index.md) action.

!syntax parameters /UserObjects/AbaqusUELStepUserObject

!syntax inputs /UserObjects/AbaqusUELStepUserObject

!syntax children /UserObjects/AbaqusUELStepUserObject
