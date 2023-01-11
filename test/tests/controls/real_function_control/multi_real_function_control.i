###########################################################
# This is a test of the Control Logic System. This test
# uses the RealFunctionControl to change a multiple Kernel
# coefficients based on an analytical function at the end
# of each timestep.
#
# @Requirement F8.10
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

[Kernels]
  [./diff_u]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  [../]
  [./time_u]
    type = TimeDerivative
    variable = u
  [../]
  [./diff_v]
    type = CoefDiffusion
    variable = v
    coef = 0.2
  [../]
  [./time_v]
    type = TimeDerivative
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
  num_steps = 5
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  csv = true
[]

[Functions]
  [./func_coef]
    type = ParsedFunction
    expression = '2*t + 0.1'
  [../]
[]

[Postprocessors]
  [./u_coef]
    type = RealControlParameterReporter
    parameter = 'Kernels/diff_u/coef'
  [../]
  [./v_coef]
    type = RealControlParameterReporter
    parameter = 'Kernels/diff_v/coef'
  [../]
[]

[Controls]
  [./func_control]
    type = RealFunctionControl
    parameter = '*/*/coef'
    function = 'func_coef'
    execute_on = 'timestep_begin'
  [../]
[]
