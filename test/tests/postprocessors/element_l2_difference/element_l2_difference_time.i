[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
[]

[Problem]
  type = FEProblem
  solve = false
[]

[AuxVariables]
  [u]
  []
[]

[ICs]
  [u]
    type = FunctionIC
    variable = u
    function = u
  []
[]

[Functions]
  [u]
    type = ParsedFunction
    value = 'sin(x + t)'
  []
[]

[AuxKernels]
  [u]
    type = FunctionAux
    variable = u
    function = u
  []
[]

[Executioner]
  type = Transient
  num_steps = 4
[]

[Postprocessors]
  [diff]
    type = ElementL2Difference
    variable = u
    other_variable = u
  []
  [diff_at_t1] # at t = 1, this should match 'diff'
    type = ElementL2Error
    variable = u
    function = 'sin(x + 0)'
  []
  [diff_at_t2] # at t = 2, this should match 'diff'
    type = ElementL2Error
    variable = u
    function = 'sin(x + 1)'
  []
  [diff_at_t3] # at t = 3, this should match 'diff'
    type = ElementL2Error
    variable = u
    function = 'sin(x + 2)'
  []
  [diff_at_t4] # at t = 4, this should match 'diff'
    type = ElementL2Error
    variable = u
    function = 'sin(x + 3)'
  []
[]

[Outputs]
  csv = true
[]
