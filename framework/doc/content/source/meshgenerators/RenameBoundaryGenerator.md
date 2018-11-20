# RenameBoundaryGenerator

!syntax description /MeshGenerators/RenameBoundaryGenerator

`RenameBoundaryGenerator` is usually used to provide meaningful names to boundaries.
For instance

```text
old_boundary_id = '1 2 3'
new_boundary_name = 'right top left'
```

Then the MOOSE input file can employ `boundary = right` rather than `boundary = 1`.  `RenameBoundaryGenerator` may also
be used to provide more meaningful names to boundaries that are already named.  For instance

```text
old_boundary_name = 'silly meaningless crazy'
new_boundary_name = 'right top left'
```

Then the MOOSE input file can employ `boundary = left` rather than `block = crazy`.

!alert warning
`RenameBoundaryGenerator` may also be used to merge boudaries, but care must be taken.

For instance

```
old_boundary_id = '1 2 3'
new_boundary_id = '4 4 4'
```

Then boudaries 1, 2 and 3 will be merged together into one boundary that may be used in the remainder of
the input file.  However, when merging boundaries problems and even inconsistencies can occur.

Firstly, in the example just given, what if the boundaries 1, 2 and 3 were named?  What should the name
of the boundary 4 be?  The convention is that it is the name of the first old boundary that is given the
boundary ID of 4, which is the name of the old boundary 1 in this case.  The user needs to be aware of this
convention.  Similarly, if `old_boundary_name = 'oldA oldB'` and `new_boundary_name = 'new1 new1'`, then
the boundary ID of new1 is the boundary ID of oldA.

Secondly, in the example above, what if boundary 4 already existed?  An inconsistency could arise in the
input file, because boundary 4 is now given the name of the old boundary 1.

Thirdly, in this example `old_boundary_id = '1 2'` and `new_boundary_name = 'wheel wheel'`

what if another boundary, with a different ID, already had the name
"wheel"?  This can lead to great confusion in the MOOSE input file.

!alert note
Given all these potential problems, when merging boundaries it is strongly recommended to use just
**one** `RenameBoundaryGenerator` that includes the names or IDs of **all** the boundaries involved in the merging.
This will make the new boundary IDs and new boundary names unequivocally obvious.

!syntax parameters /MeshGenerators/RenameBoundaryGenerator

!syntax inputs /MeshGenerators/RenameBoundaryGenerator

!syntax children /MeshGenerators/RenameBoundaryGenerator
