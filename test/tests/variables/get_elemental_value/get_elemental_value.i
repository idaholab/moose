# Tests the getElementalValue function of MooseVariableFE.
#
# The tested aux copies the first elemental value of another variable. The
# setup is the following IVP:
#   du/dt = 1
#   u(0) = 0
# Therefore the solution is u(t) = t. Five time steps of dt = 1 are taken.
# The expected output for each time level is thus the following:
#   current: [0,1,2,3,4,5]
#   old:     [0,0,1,2,3,4]
#   older:   [0,0,0,1,2,3]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[Variables]
  [./copied_var]
  [../]
[]

[AuxVariables]
  [./test_var]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./test_var_aux]
    type = GetElementalValueAux
    variable = test_var
    copied_variable = copied_var
    # The parameter "time_level" is provided by tests file
  [../]
[]

[ICs]
  [./copied_var_ic]
    type = ConstantIC
    variable = copied_var
    value = 0
  [../]
[]

[Kernels]
  [./time_der]
    type = TimeDerivative
    variable = copied_var
  [../]
  [./src]
    type = BodyForce
    variable = copied_var
    function = 1
  [../]
[]

[Executioner]
  type = Transient
  scheme = implicit-euler
  dt = 1
  num_steps = 5
  abort_on_solve_fail = true

  solve_type = NEWTON
[]

[Postprocessors]
  [./test_pp]
    type = ElementAverageValue
    variable = test_var
  [../]
[]

[Outputs]
  csv = true
[]
