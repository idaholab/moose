[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./var]
    outputs = none
  [../]
[]

[AuxVariables]
  [u]
  []
[]

[AuxKernels]
  [u]
    type = FunctionAux
    function = func
    variable = u
  []
[]

[Functions]
  [func]
    type = ParsedFunction
    value = 'x*y*(t+3)'
  []
[]

[ICs]
  [u]
    type = FunctionIC
    function = func
    variable = u
  []
[]

[Problem]
  type = FEProblem
  solve = false
  kernel_coverage_check = false
[]


[Executioner]
  type = Transient
  start_time = -7
  num_steps = 2
  dt = 9
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = TIMESTEP_END
  []
[]
