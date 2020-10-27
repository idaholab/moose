[Mesh]
  [generate]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
  [new_block]
    type = SubdomainBoundingBoxGenerator
    input = generate
    bottom_left = '0.25 0.25 0'
    top_right = '0.75 0.75 0'
    block_id = 1980
  []
[]

[Variables]
  [not_u]
  []
[]

[AuxVariables]
  [aux]
    initial_condition = 1980
  []
  [u]
    initial_condition = 624
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = not_u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = not_u
    boundary = left
    value = 4
  []
  [right]
    type = DirichletBC
    variable = not_u
    boundary = right
    value = 6
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
