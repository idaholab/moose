[Mesh]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []

  [./lower_d_block]
    type = LowerDBlockFromSidesetGenerator
    input = gmg
    new_block_id = 10
    sidesets = 'bottom bottom right top left'
  []
[]

[Variables]
  [./u]
    block = 0
  [../]
  [./v]
    block = 10
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
    block = 0
  [../]
  [./srcv]
    type = BodyForce
    block = 10
    variable = v
    function = 1
  [../]
  [./time_v]
    type = TimeDerivative
    block = 10
    variable = v
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 2
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
