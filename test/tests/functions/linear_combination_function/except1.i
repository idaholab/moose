# LinearCombinationFunction function test
# See [Functions] block for a description of the tests

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 2
  nx = 10
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
  [./the_linear_combo]
  [../]
[]

[AuxKernels]
  [./the_linear_combo]
    type = FunctionAux
    variable = the_linear_combo
    function = the_linear_combo
  [../]
[]


[Functions]
  [./twoxplus1]
    type = ParsedFunction
    expression = 2*x+1
  [../]

  [./xsquared]
    type = ParsedFunction
    expression = x*x
  [../]

  [./the_linear_combo]
    type = LinearCombinationFunction
    functions = 'x twoxplus1 xsquared'
    w = '0.5 5 0.4 0.3'
  [../]

  [./should_be_answer]
    type = ParsedFunction
    expression = 0.5*x+5*(2*x+1)*0.4*x*x+0.3*7
  [../]
[]

[Postprocessors]
  [./should_be_zero]
    type = NodalL2Error
    function = should_be_answer
    variable = the_linear_combo
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.5
  end_time = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  hide = dummy
  exodus = false
  csv = true
[]
