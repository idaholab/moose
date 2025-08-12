# AbaqusUELInitialCondition

!syntax description /UserObjects/AbaqusUELInitialCondition

## Description

`AbaqusUELInitialCondition` applies field initial conditions from the Abaqus input loaded by an
[AbaqusUELMesh](mesh/AbaqusUELMesh.md). It runs at `INITIAL` time and directly writes nodal
values for the corresponding variables on the nodes indicated by the Abaqus node sets.

- Retrieves initial condition entries (`var_id`, node set names, values) from the UEL mesh.
- Maps Abaqus variable IDs to MOOSE variable names via `AbaqusUELMesh::getVarName`.
- For each node in each referenced node set, sets the initial nodal value of the mapped variable.
- Requires the mesh to be an `AbaqusUELMesh` and variables present (e.g., added via
  [AddUELVariables](actions/AddUELVariables.md)).

### Usage

Example of enabling initial conditions via this user object:

!listing modules/solid_mechanics/test/tests/uel_mesh/elastic.i block=UserObjects

!syntax parameters /UserObjects/AbaqusUELInitialCondition

!syntax inputs /UserObjects/AbaqusUELInitialCondition

!syntax children /UserObjects/AbaqusUELInitialCondition

!bibtex bibliography

