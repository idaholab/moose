[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4

  xmax = 1.2
  ymax = 2.3
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

[Postprocessors]
  [./right]
    type = AreaPostprocessor
    boundary = 'right'
    execute_on = 'initial timestep_end'
  [../]

  [./bottom]
    type = AreaPostprocessor
    boundary = 'bottom'
    execute_on = 'initial timestep_end'
  [../]

  [./all]
    type = AreaPostprocessor
    boundary = 'left right bottom top'
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  csv = true
[]
