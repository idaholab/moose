# RenameBoundaryGenerator

!syntax description /Mesh/RenameBoundaryGenerator

## Renaming or Setting Boundary Names

When using the `RenameBoundaryGenerator` to change boundary names, the result is independent of ordering.

The following will change the name for the boundary "meaningless" to "interior" and the name for boundary "5" to "exterior":

```
[rename]
  type = RenameBoundaryGenerator
  input = some_mesh
  old_boundary = 'meaningless 5'
  new_boundary = 'interior exterior'
[]
```

For the special case where the original boundary name and ID are the same, this mesh generator
will convert *both* to the new boundary name/ID. For instance, if the mesh contains a boundary
named `3` that also has an ID of `3`, this mesh generator will convert both the name and ID.

## Merging Boundaries

When using the `RenameBoundaryGenerator` to merge boundaries, the result is not necessarily independent of ordering.

For example, take the following:

```
[merge]
  type = RenameBoundaryGenerator
  input = some_mesh
  old_boundary = '0 1 2 3`
  new_boundary = 'bottom_and_left bottom_and_left top_and_right top_and_right'
[]
```

The above will result in two boundaries:

- Boundary "0" with name "bottom_and_left" that contains the sides from the original boundaries "0" and "1".
- Boundary "2" with name "top_and_right" that contains the sides from the original boundaries "2" and "3".

Take the first execution, "0" to "bottom_and_left". The "RenameBoundaryGenerator" will use the original boundary ID, which is "0". The second execution, "1" to "bottom_and_left", will use the new ID associated with "bottom_and_left", which is "0", and merge "1" into it. The result is similar for the third and fourth executions.

!alert! tip title=The use of ID is order independent
The order dependent behavior only exists when the new boundary provided is a name. Take the following instead:

```
[merge]
  type = RenameBoundaryGenerator
  input = some_mesh
  old_boundary = '0 1 2 3'
  new_boundary = '0 0 4 4'
[]
```

The result is:

- Boundary "0" that contains the sides from original boundaries "0" and "1".
- Boundary "4" that contains the sides from original boundaries "2" and "3".
!alert-end!

!syntax parameters /Mesh/RenameBoundaryGenerator

!syntax inputs /Mesh/RenameBoundaryGenerator

!syntax children /Mesh/RenameBoundaryGenerator
