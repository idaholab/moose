# RenameBlock
!syntax description /MeshModifiers/RenameBlock

`RenameBlock` is usually used to provide meaningful names to blocks so
that input files are easier to read.  For instance

    old_block_id = '1 2 3'
    new_block_name = 'wheel engine axle'

Then the MOOSE input file can employ `block = wheel` rather than `block =
1`.  `RenameBlock` may also be to provide more meaningful names to
mesh blocks that are already named.  For instance

    old_block_name = 'silly meaningless crazy'
    new_block_name = 'wheel engine axle'

Then the MOOSE input file can employ `block = axle` rather than `block
= crazy`.

!!! warning
    `RenameBlock` may also be used to merge blocks, but care must be
    taken.

For instance

    old_block_id = '1 2 3'
    new_block_id = '4 4 4'

Then blocks 1, 2 and 3 will be merged together into one block that may
be used in the remainder of the input file.  However, when merging
blocks problems and even inconsistencies can occur.

Firstly, in the example just given, what if the blocks 1, 2 and 3 were
named?  What should the name of the block 4 be?  The convention is
that it is the name of the first old block that is given the block ID
of 4, which is the name of the old block 1 in this case.  The user
needs to be aware of this convention.  Similarly, if `old_block_name =
'oldA oldB'` and `new_block_name = 'new1 new1'`, then the block ID of
new1 is the block ID of oldA.

Secondly, in the example above, what if block 4 already existed?  An
inconsistency could arise in the input file, because block 4 is now
given the name of the old block 1.

Thirdly, in this example

`old_block_id = '1 2'` and
`new_block_name = 'wheel wheel'`

what if another block, with a different ID, already had the name
``wheel''?  This can lead to great confusion in the MOOSE input file.

!!! info
    Given all these potential problems, when merging blocks it is
    strongly recommended to use just **one** `RenameBlock` that
    includes the names or IDs of **all** the blocks involved in the
    merging.  This will make the new block IDs and new block names
    unequivocally obvious.

!syntax parameters /MeshModifiers/RenameBlock

!syntax inputs /MeshModifiers/RenameBlock

!syntax children /MeshModifiers/RenameBlock
