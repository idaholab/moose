# RemeshCount

!syntax description /Postprocessors/RemeshCount

The count is cumulative over the run and increases by one each time the
[Remeshing system](syntax/Remeshing/index.md) actually replaces elements, which is not every time a
criterion fires: a remesher that rejects every candidate patch leaves the count unchanged.

An input whose `[Remeshing]` block is absent or commented out reports zero, so the
`[Postprocessors]` block does not have to be edited alongside the `[Remeshing]` block.

## Example Input File Syntax

!listing test/tests/remeshing/ale_remesh_reset.i block=Postprocessors

!syntax parameters /Postprocessors/RemeshCount

!syntax inputs /Postprocessors/RemeshCount

!syntax children /Postprocessors/RemeshCount
