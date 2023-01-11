# LinearCombinationFunction function test
# See [Functions] block for a description of the tests

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  nx = 3
  ny = 3
[]

[Variables]
  [./dummy]
  [../]
[]

[Kernels]
  [./dummy_u]
    type = TimeDerivative
    variable = dummy
  [../]
[]


[AuxVariables]
  [./the_linear_combo_x]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./the_linear_combo_y]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./the_linear_combo_x]
    type = FunctionDerivativeAux
    component = x
    variable = the_linear_combo_x
    function = the_linear_combo
  [../]
  [./the_linear_combo_y]
    type = FunctionDerivativeAux
    component = y
    variable = the_linear_combo_y
    function = the_linear_combo
  [../]
[]


[Functions]
  [./xtimes]
    type = ParsedGradFunction
    value = '1.1*x+y'
    grad_x = '1.1'
    grad_y = '1'
  [../]

  [./twoxplus1]
    type = ParsedGradFunction
    value = '2*x+1'
    grad_x = '2'
  [../]

  [./tover2]
    type = ParsedGradFunction
    value = '0.5*t-y*7'
    grad_y = '-7'
  [../]

  [./the_linear_combo]
    type = LinearCombinationFunction
    functions = 'xtimes twoxplus1 tover2'
    w = '3 -1.2 3'
  [../]

  [./should_be_answer_x]
    type = ParsedFunction
    expression = '3*1.1-1.2*2'
  [../]
  [./should_be_answer_y]
    type = ParsedFunction
    expression = '3*1+3*(-7)'
  [../]
[]

[Postprocessors]
  [./should_be_zero_x]
    type = ElementL2Error
    function = should_be_answer_x
    variable = the_linear_combo_x
  [../]
  [./should_be_zero_y]
    type = ElementL2Error
    function = should_be_answer_y
    variable = the_linear_combo_y
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.5
  end_time = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = lcf_grad
  hide = dummy
  exodus = false
  csv = true
[]
