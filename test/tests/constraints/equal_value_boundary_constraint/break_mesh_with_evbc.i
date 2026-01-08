[Mesh]
  [file]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
  []
  [mirror]
    type = SymmetryTransformGenerator
    input = file
    mirror_point = "0 1 0"
    mirror_normal_vector = "0 1 0"
  []
  [cmbn]
    type = CombinerGenerator
    inputs = 'file mirror'
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
    boundary = 'left'
    value = 0.0
  []
  [right]
    type = DirichletBC
    variable = diffused
    preset = false
    boundary = 'right'
    value = 0.0
  []
[]

[Constraints]
  [corner]
    type = EqualValueBoundaryConstraint
    variable = diffused
    secondary_node_ids = '1 2 3 4 7 8'
    primary_node_coord = '1 2 0'
    penalty = 10e6
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = diffused
  []
[]
