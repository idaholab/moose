[Mesh]
  file = rz_xyz.e
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./one]
    initial_condition = 1
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
  [./left_middle]
    type = DirichletBC
    variable = u
    boundary = left_middle
    value = 1
  [../]
  [./right_middle]
    type = DirichletBC
    variable = u
    boundary = right_middle
    value = 0
  [../]
[]

[Postprocessors]
  [./volume]
    type = ElementIntegralVariablePostprocessor
    variable = one
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = out
  exodus = true
[]

[Problem]
  coord_type = 'RZ XYZ'
  block = '1 2'
[]
