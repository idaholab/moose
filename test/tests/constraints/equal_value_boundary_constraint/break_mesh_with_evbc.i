[Problem]
  boundary_restricted_node_integrity_check = false
[]

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = ../../meshgenerators/break_mesh_by_block_generator/4ElementJunction.e
  []

  [breakmesh]
    type = BreakMeshByBlockGenerator
    block_pairs = '2 4'
    input = fmg
  []

  [add_surfaces]
    type = SideSetsFromNormalsGenerator
    input = breakmesh
    normals = '0  1  0
               1  0  0
               0 -1  0
              -1  0  0'
    fixed_normal = true
    new_boundary = 'y1 x1 y0 x0'
  []
[]

[Outputs]
  exodus = true
[]

[Variables]
  [diffused]
    order = FIRST
  []
[]

[Executioner]
  type = Steady

  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type -sub_pc_type'
  petsc_options_value = 'asm       lu'

  line_search = 'none'
[]

[BCs]
  [left]
    type = DirichletBC
    variable = diffused
    preset = false
    boundary = 'x0'
    value = 1.0
  []
  [right]
    type = DirichletBC
    variable = diffused
    preset = false
    boundary = 'x1'
    value = 0.0
  []
[]

[Constraints]
  [corner]
    type = EqualValueBoundaryConstraint
    variable = diffused
    secondary_node_ids = '1 2 3 4 5 6'
    primary_node_coord = '-0.5 0.5 0'
    penalty = 10e6
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = diffused
  []
[]
