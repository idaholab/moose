# CombinerGenerator

!syntax description /Mesh/CombinerGenerator

## Overview

The `CombinerGenerator` allows the user to combine the outputs of multiple `MeshGenerator`s into a single mesh.  This is somewhat similar to the [StitchedMeshGenerator.md] with the difference being that `CombinerGenerator` makes no attempt to "heal" / "join" the mesh like [StitchedMeshGenerator.md] does.  There `CombinerGenerator` is more suited to creation of disjoint meshes (where the individual pieces are not directly tied together).

!alert note title=Mesh naming precedence
`CombinerGenerator` preserves subdomain names and boundary names (node sets, side sets, and edge sets).
If the corresponding IDs exist in multiple meshes, then the meshes/copies listed
later in [!param](/Mesh/CombinerGenerator/inputs)/[!param](/Mesh/CombinerGenerator/positions)
take precedence.

## Usage

There are three main ways to use the `CombinerGenerator`:

### 1. Combine Multiple `MeshGenerator`s

The most straightforward thing to do is simply to combine the output of multiple `MeshGenerator`s together into a single mesh.  For example:

!listing test/tests/meshgenerators/combiner_generator/combiner_multi_input.i
         block=Mesh

Will generate a mesh that looks like:

!media media/meshgenerators/combiner_multi.png style=width:50%;


### 2. Combine Multiple `MeshGenerator`s AND Translate Them

It is also possible to translate (move) the input `MeshGenerator`s as they are combined.  This is done using the `positions` option which takes triplets of floating point numbers that are interpreted as displacement vectors for moving each of the input meshes.

!alert note
If you specify `positions` then the number of `positions` must match the number of `inputs`, unless only one input is specified (more on that in a moment).

!listing test/tests/meshgenerators/combiner_generator/combiner_multi_input_translate.i
         block=Mesh

Will generate a mesh that looks like:

!media media/meshgenerators/combiner_multi_translate.png style=width:75%;

Alternatively, the same displacement vectors can be supplied in a file with the `positions_file` option. The above mesh can equivalently be generated with the following.

!listing moose/test/tests/meshgenerators/combiner_generator/combiner_multi_input_translate_from_file.i
         block=Mesh

where the `positions.txt` file contains the floating point triplets.

!listing moose/test/tests/meshgenerators/combiner_generator/positions.txt

The same restrictions on `positions` also apply to the number of entries in `position_file`.

### 3. Copy a Single Input Multiple Times With Translations

The final option is to provide exactly one `inputs` but specify multiple `positions`.  This will cause the single input to be copied multiple times with the position of each copy specified by the `positions` parameter.  For example

!listing test/tests/meshgenerators/combiner_generator/combiner_generator.i
         block=Mesh

Will generate a mesh that looks like:

!media media/meshgenerators/combiner.png style=width:75%;

Again, the same capability can be achieved with the `positions_file` option.

!syntax parameters /Mesh/CombinerGenerator

!syntax inputs /Mesh/CombinerGenerator

!syntax children /Mesh/CombinerGenerator
