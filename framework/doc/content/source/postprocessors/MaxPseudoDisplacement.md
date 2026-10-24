# MaxPseudoDisplacement

!syntax description /Postprocessors/MaxPseudoDisplacement

The reported value is $\max_n \lVert \mathbf{d}_n \rVert$ over the nodes of the mesh, where
$\mathbf{d}_n$ is the pseudo-displacement of node $n$ accumulated since the last mesh replacement.
It returns to zero on the step a replacement occurs, because the
[Remeshing system](syntax/Remeshing/index.md) adopts the newly built mesh as the reference
configuration at that step. See [the reference configuration](syntax/Remeshing/index.md#motion).

The pseudo-displacement is identically zero unless
[!param](/Remeshing/RemeshingAction/mesh_movement) is `true`. An input that does not move the mesh,
or that has no `[Remeshing]` block at all, reports zero rather than an error.

## Example Input File Syntax

!listing test/tests/remeshing/ale_remesh_reset.i block=Postprocessors

!syntax parameters /Postprocessors/MaxPseudoDisplacement

!syntax inputs /Postprocessors/MaxPseudoDisplacement

!syntax children /Postprocessors/MaxPseudoDisplacement
