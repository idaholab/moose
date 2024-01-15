[Mesh]
  [bulk]
    type = GeneratedMeshGenerator # Can generate simple lines, rectangles and rectangular prisms
    dim = 3
    nx = 12
    ny = 16
    nz = 5
    xmax = 10 # m
    ymax = 13 # m
    zmax = 8 # m
  []

  # we removed the water for simplicity
  [add_inner_concrete]
    type = ParsedSubdomainMeshGenerator
    input = bulk
    block_id = 2
    combinatorial_geometry = 'x > 2 & x < 7.5 & y > 2 & y < 11 & z > 1 & z < 5'
  []
  [hollow_concrete]
    type = ParsedSubdomainMeshGenerator
    input = add_inner_concrete
    block_id = 1
    combinatorial_geometry = 'x > 3.5 & x < 6 & y > 3.5 & y < 9.5 & z > 1 & z < 5'
  []

  [rename_blocks]
    type = RenameBlockGenerator
    input = hollow_concrete
    old_block = '0 1 2'
    new_block = 'concrete cavity concrete_inner'
  []

  [add_inner_cavity]
    type = SideSetsBetweenSubdomainsGenerator
    input = rename_blocks
    primary_block = 'concrete_inner concrete'
    paired_block = cavity
    new_boundary = 'inner_cavity'
  []

  [add_concrete_outer_boundary]
    type = RenameBoundaryGenerator
    input = add_inner_cavity
    old_boundary = 'left right front bottom top back'
    new_boundary = 'air_boundary air_boundary air_boundary air_boundary air_boundary ground'
  []

  [remove_cavity]
    type = BlockDeletionGenerator
    input = 'add_concrete_outer_boundary'
    block = cavity
  []
[]

# To avoid erroring if the user forgets --mesh-only
[Problem]
  solve = false
[]
[AuxVariables]
  [placeholder]
  []
[]
[Executioner]
  type = Steady
[]
[Outputs]
  [exodus]
    type = Exodus
    file_base = 'mesh_coarse_in'
  []
[]
