[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
  [./v]
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
  [./diff_v]
    type = CoefDiffusion
    variable = v
    coef = 2
  [../]
[]

[BCs]
  [./right_u]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 3
  [../]
  [./left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 2
  [../]
[]

[Postprocessors]
  [./var1]
    type = NumVars
    system = 'NL'
  [../]
  [./var2]
    type = NumVars
    system = 'NL'
    outputs = 'console'
    execute_on = 'timestep_begin timestep_end'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  execute_on = 'timestep_end'
  csv = true
[]
