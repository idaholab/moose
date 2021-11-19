# RenameBlockGenerator

!syntax description /Mesh/RenameBlockGenerator

## Renaming or Setting Block Names

When using the `RenameBlockGenerator` to change block names, the result is independent of ordering.

The following will change the name for the block `meaningless` to `inside` and will set the name for block `5` to `outside`:

```
[rename]
  type = RenameBlockGenerator
  input = some_mesh
  old_block = 'meaningless 5'
  new_block = 'inside outside'
[]
```

## Merging Blocks

The `RenameBlockGenerator` can be used to merge blocks together. The result is independent of ordering when [!param](/Mesh/RenameBlockGenerator/new_block) contains only block IDs. When [!param](/Mesh/RenameBlockGenerator/new_block) contains block names, the result is not necessarily independent of ordering.

The following will result in the merging of the elements in blocks `1`, `2`, and `3` into block `0`:

```
[merge]
  type = RenameBlockGenerator
  input = some_mesh
  old_block = '1 2 3'
  new_block = '0 0 0'
[]
```

As discussed previously, when providing names in [!param](/Mesh/RenameBlockGenerator/new_block), the result may be ordering-dependent. If a name is provided in [!param](/Mesh/RenameBlockGenerator/new_block) that does not already exist in the mesh, it will take the ID of the first rename.

Take the following examples (assuming that the block `some_block` does not exist yet):

```
[mergename0]
  type = RenameBlockGenerator
  input = some_mesh
  old_block = '0 1'
  new_block = 'some_block some_block'
[]
```

```
[mergename1]
  type = RenameBlockGenerator
  input = some_mesh
  old_block = '1 0'
  new_block = 'some_block some_block'
[]
```

The result of each will be a block named `some_block` that contains elements that were in blocks `0` and `1`. However, because the block `some_block` takes the ID of the first rename execution, the ID of `some_block` will be `0` in the case of the generator `mergename0` and `1` in the case of the generator `mergename1`.

In the case of providing [!param](/Mesh/RenameBlockGenerator/new_block) by ID, the following examples (assuming that the block `2` does not exist yet) will have the same result:

```
[mergeblock0]
  type = RenameBlockGenerator
  input = some_mesh
  old_block = '0 1'
  new_block = '2 2'
[]
```

```
[mergeblock1]
  type = RenameBlockGenerator
  input = some_mesh
  old_block = '1 0'
  new_block = '2 2'
[]
```

That is, a block will be created with the ID `2` that contains the elements previously in blocks `0` and `1`.

In summary, the possibility of ordering-dependent results with the `RenameBlockGenerator` depends on whether or not name(s) are provided in [!param](/Mesh/RenameBlockGenerator/new_block) and said name(s) do not exist yet.

!syntax parameters /Mesh/RenameBlockGenerator

!syntax inputs /Mesh/RenameBlockGenerator

!syntax children /Mesh/RenameBlockGenerator
