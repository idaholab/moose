# RenameBlockGenerator

!syntax description /Mesh/RenameBlockGenerator

## Renaming or Setting Block Names

When using the `RenameBlockGenerator` to change block names, the result is independent of ordering.

The following will change the name for the block "meaningless" to "inside" and will set the name for block "5" to "outside":

```
[rename]
  type = RenameBlockGenerator
  input = some_mesh
  old_block = 'meainingless 5'
  new_block = 'inside outside'
[]
```

## Merging Blocks

When using the `RenameBlockGenerator` to merge blocks, the result is not necessarily independent of ordering.

For example, take the following:

```
[merge]
  type = RenameBlockGenerator
  input = some_mesh
  old_block = '0 1 2 3'
  new_block = '0_and_1 0_and_1 2_and_3 2_and_3'
[]
```

The above will result in two blocks:

- Block "0_and_1" with ID "0" that contains the elements from the original blocks "0" and "1".
- Block "2_and_3" with ID "2" that contains the elements from the original blocks "2" and "3".

Take the first execution, "0" to "0_and_1". The `RenameBlockGenerator` will use the original block ID, which is "0". The second execution "1" to "0_and_1" will use the new ID associated with "0_and_1", which is "0", and merge "1" into it. The result is similar for the third and fourth executions.

!alert! tip title=The use of ID is order independent
The order dependent behavior only exists when the new block provided is a name. Take the following instead:

```
[merge]
  type = RenameBlockGenerator
  input = some_mesh
  old_block = '0 1 2 3'
  new_block = '0 0 4 4'
[]
```

The result is:

- Block "0" that contains the elements from original blocks "0" and "1".
- Block "4" that contains the elements from original blocks "2" and "3".
!alert-end!

!syntax parameters /Mesh/RenameBlockGenerator

!syntax inputs /Mesh/RenameBlockGenerator

!syntax children /Mesh/RenameBlockGenerator
