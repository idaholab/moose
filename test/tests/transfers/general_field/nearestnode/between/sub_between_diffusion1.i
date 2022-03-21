[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
  [block1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '0.5 0.5 0'
  []
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./transferred_u]
  [../]
  [./elemental_transferred_u]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 3
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 4
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  execute_on = 'final'
[]
