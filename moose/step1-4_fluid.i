!include step1-2_concentric_circle.i

[Mesh]
  # Remove block 1 (hole), 2 (fuel), 3 (clad)
  # and define a boundary "inner" on the interface
  # (the inside)
  [remove_fuel_pin]
    type = BlockDeletionGenerator
    input = concentric_circle
    block = '1 2 3'
    new_boundary = inner
  []

  # Merge the remaining blocks (4 - water boundary layer,
  # 5 - water bulk) into a single block "water"
  [rename_blocks]
    type = RenameBlockGenerator
    input = remove_fuel_pin
    old_block = '4 5'
    new_block = 'water water'
  []
[]
