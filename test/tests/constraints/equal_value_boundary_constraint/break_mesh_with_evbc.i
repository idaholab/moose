[Problem]
  boundary_restricted_node_integrity_check = false
  solve = false
[]

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = ../../meshgenerators/break_mesh_by_block_generator/4ElementJunction.e
  []

  [breakmesh]
    type = BreakMeshByBlockGenerator
    input = fmg
    split_interface = true
    add_interface_on_two_sides = true
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
  parallel_type = REPLICATED
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
    secondary_node_ids = '5 10'
    primary_node_coord = '0.5 0.5 0.0'
    penalty = 10e6
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = diffused
  []
[]
