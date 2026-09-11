# FlattenRefinementGenerator

!syntax description /Mesh/FlattenRefinementGenerator

## Overview

The `FlattenRefinementGenerator` takes a mesh that has been h-refined (for example by
[RefineBlockGenerator.md] or [RefineSidesetGenerator.md]) and discards its refinement tree,
keeping only the finest (active) elements and turning them into ordinary, unrefined level-0
elements. The resulting mesh no longer carries any refinement pattern and can be treated as a
fresh initial mesh.

When [!param](/Mesh/FlattenRefinementGenerator/first_order) is set to `true`, the flattened mesh
is additionally converted to first order, equivalent to a `FIRST_ORDER` conversion with
[ElementOrderConversionGenerator.md]. Otherwise the element order of the input mesh is preserved.

!alert note
Flattening only makes sense for a mesh that has been refined. Because the refinement tree is
discarded, a locally (adaptively) refined mesh will retain the hanging nodes it had at its finest
level, producing a non-conforming mesh. This generator is therefore most useful for uniformly
refined meshes.

## Example Input Syntax

!listing test/tests/meshgenerators/flatten_refinement_generator/flatten_refinement.i block=Mesh

!syntax parameters /Mesh/FlattenRefinementGenerator

!syntax inputs /Mesh/FlattenRefinementGenerator

!syntax children /Mesh/FlattenRefinementGenerator
