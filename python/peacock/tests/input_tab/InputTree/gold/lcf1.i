# LinearCombinationFunction function test
# See [Functions] block for a description of the tests
[AuxKernels]
  [./the_linear_combo]
    type = FunctionAux
    function = the_linear_combo
    variable = the_linear_combo
  [../]
[]

[AuxVariables]
  [./the_linear_combo]
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.5
  end_time = 1
[]

[Functions]
  [./xtimes]
    type = ParsedFunction
    value = '1.1*x'
  [../]
  [./twoxplus1]
    type = ParsedFunction
    value = '2*x+1'
  [../]
  [./xsquared]
    type = ParsedFunction
    value = '(x-2)*x'
  [../]
  [./tover2]
    type = ParsedFunction
    value = '0.5*t'
  [../]
  [./the_linear_combo]
    type = LinearCombinationFunction
    functions = 'xtimes twoxplus1 xsquared tover2'
    w = '3 -1.2 0.4 3'
  [../]
  [./should_be_answer]
    type = ParsedFunction
    value = '3*1.1*x-1.2*(2*x+1)+0.4*(x-2)*x+3*0.5*t'
  [../]
[]

[Kernels]
  [./dummy_u]
    type = TimeDerivative
    variable = dummy
  [../]
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmax = 2
  xmin = 0
[]

[Outputs]
  csv = true
  execute_on = 'timestep_end'
  exodus = false
  file_base = lcf1
  hide = 'dummy'
[]

[Postprocessors]
  [./should_be_zero]
    type = NodalL2Error
    function = should_be_answer
    variable = 'the_linear_combo'
  [../]
[]

[Variables]
  [./dummy]
  [../]
[]

