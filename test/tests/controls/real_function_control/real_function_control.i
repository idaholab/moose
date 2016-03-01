###########################################################
# This is a test of the Control Logic System. This test
# uses the RealFunctionControl to change a Kernel
# coefficient based on an analytical function at the end
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
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  [../]
  [./time]
    type = TimeDerivative
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

[Executioner]
  # Preconditioned JFNK (default)
  type = Transient
  num_steps = 10
  dt = 0.1
  dtmin = 0.1
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
    value = '2*t + 0.1'
  [../]
[]

[Postprocessors]
  [./coef]
    type = RealControlParameterReporter
    parameter = 'Kernels/diff/coef'
  [../]
[]

[Controls]
  [./func_control]
    type = RealFunctionControl
    parameter = 'coef'
    function = 'func_coef'
    execute_on = 'initial timestep_begin'
  [../]
[]
