[Mesh]
  type = FileMesh
  file = rectangle.e
  dim = 2
[]

[Variables]
  [./u]
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
    boundary = 1
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Postprocessors]
  [./all_pp]
    type = NumInternalSides
    execute_on = 'initial timestep_end'
   [../]
  [./block_1_pp]
    type = NumInternalSides
    block = 1
    execute_on = 'initial timestep_end'
  [../]
  [./block_2_pp]
    type = NumInternalSides
    block = 2
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = false
  csv = true
[]
