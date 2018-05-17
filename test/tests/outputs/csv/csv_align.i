[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./aux0]
    order = SECOND
    family = SCALAR
  [../]
  [./aux1]
    family = SCALAR
    initial_condition = 5
  [../]
  [./aux2]
    family = SCALAR
    initial_condition = 10
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
  [./time]
    type = TimeDerivative
    variable = u
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

[]

[Postprocessors]
  [./num_vars]
    type = NumVars
    system = 'NL'
  [../]
  [./num_aux]
    type = NumVars
    system = 'AUX'
  [../]
  [./norm]
    type = ElementL2Norm
    variable = u
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 4
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  execute_on = 'timestep_end'
  [./out]
    type = CSV
    align = true
    delimiter = ', '
    sync_times = '0.123456789123412 0.15 0.2'
    precision = 8
  [../]
[]

[ICs]
  [./aux0_IC]
    variable = aux0
    values = '12 13'
    type = ScalarComponentIC
  [../]
[]
