# Include Mesh/concentric_circle, which builds the
# full concentric circle mesh
!include step1-2_concentric_circle.i

[Mesh]
  # Remove block 1 (hole) and define a boundary
  # "inner" on the interface (the inside)
  [remove_hole]
    type = BlockDeletionGenerator
    input = concentric_circle
    block = 1
    new_boundary = inner
  []

  # Remove block 4 (water boundary layer) and
  # block 5 (water bulk) and define a boundary
  # "water_solid_interface" on the interface
  # (the outside)
  [remove_water]
    type = BlockDeletionGenerator
    input = remove_hole
    block = '4 5'
    new_boundary = water_solid_interface
  []

  # Rename the remaining blocks (2: fuel, 3: clad)
  [rename_blocks]
    type = RenameBlockGenerator
    input = remove_water
    old_block = '2 3'
    new_block = 'fuel clad'
  []
[]
