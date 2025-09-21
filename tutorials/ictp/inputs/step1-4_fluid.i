# Include Mesh/concentric_circle, which builds the
# full concentric circle mesh
!include step1-2_concentric_circle.i

[Mesh]
  # Remove block 1 (hole), 2 (fuel), 3 (clad)
  # and define a boundary "water_solid_interface"
  # on the interface (the inside)
  [remove_fuel_pin]
    type = BlockDeletionGenerator
    input = concentric_circle
    block = '1 2 3'
    new_boundary = water_solid_interface
  []

  # Merge the remaining blocks (4 - water boundary layer,
  # 5 - water bulk) into a single block "water"
  [rename_blocks]
    type = RenameBlockGenerator
    input = remove_fuel_pin
    old_block = '4 5'
    new_block = 'water water'
  []

  # Rename and merge the outer boundaries created by
  # the ConcentricCircleMeshGenerator (top, right,
  # bottom, left) into a single boundary (water)
  [merge_outer]
    type = RenameBoundaryGenerator
    input = rename_blocks
    old_boundary = 'top right bottom left'
    new_boundary = 'outer outer outer outer'
  []
[]
