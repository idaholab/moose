# CombinerGenerator

!syntax description /MeshGenerators/CombinerGenerator

## Description

The `CombinerGenerator` allows the user to combine the outputs of multiple `MeshGenerator`s into a single mesh.  This is somewhat similar to [StitchedMesh.md] with the difference being that `CombinerGenerator` makes no attempt to "heal" / "join" the mesh like [StitchedMesh.md].  There `CombinerGenerator` is more suited to creation of disjoint meshes (where the individual pieces are not directly tied together).

## Usages

There are three main ways to use the `CombinerGenerator`:

### 1. Combine Multiple `MeshGenerator`s

The most straightforwad thing to do is simply to combine the output of multiple `MeshGenerator`s together into a single mesh.  For example:

!listing moose/test/tests/meshgenerators/combiner_generator/combiner_multi_input.i
         block=MeshGenerators

Will generate a mesh that looks like:

!media media/meshgenerators/combiner_multi.png style=width:50%;


### 2. Combine Multiple `MeshGenerator`s AND Translate Them

It is also possible to translate (move) the input `MeshGenerator`s as they are combined.  This is done using the `positions` option which takes triplets of floating point numbers that are interpreted as displacement vectors for moving each of the input meshes.

!alert note
If you specify `positions` then the number of `positions` must match the number of `inputs`, unless only one input is specified (more on that in a moment).

!listing moose/test/tests/meshgenerators/combiner_generator/combiner_multi_input_translate.i
         block=MeshGenerators

Will generate a mesh that looks like:

!media media/meshgenerators/combiner_multi_translate.png style=width:75%;

### 3. Copy a Single Input Multiple Times With Translations

The final option is to provide exactly one `inputs` but specify multiple `positions`.  This will cause the single input to be copied multiple times withthe position of each copy specified by the `positions` parameter.  For example

!listing moose/test/tests/meshgenerators/combiner_generator/combiner_generator.i
         block=MeshGenerators

Will generate a mesh that looks like:

!media media/meshgenerators/combiner.png style=width:75%;

!syntax parameters /MeshGenerators/CombinerGenerator

!syntax inputs /MeshGenerators/CombinerGenerator

!syntax children /MeshGenerators/CombinerGenerator

!bibtex bibliography
