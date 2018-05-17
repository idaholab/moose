###########################################################
# This test exercises console Output control. Various
# controls are implemented using this input file including
# turning off color, changing Postprocessor output,
# toggling the performance logging, and verifying
# simulation information on the console.
#
# @Requirement U1.40
###########################################################


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
  [./num_vars]
    type = NumVars
    system = 'NL'
  [../]
  [./num_aux]
    type = NumVars
    system = 'AUX'
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
  color = false
  [./screen]
    type = Console
    fit_mode = 40
  [../]
[]

[ICs]
  [./aux0_IC]
    variable = aux0
    values = '12 13'
    type = ScalarComponentIC
  [../]
[]
