[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 4
  []
[]

[Functions]
  [test]
    type = ParsedFunction
    expression = 't + x^2 + y^3 + sin(5*z)'
  []
[]

[AuxVariables]
  [a]
  []
  [b]
  []
  [c]
  []
  [d]
  []
[]

[ICs]
  [a]
    type = FunctionIC
    variable = a
    function = x
  []
  [b]
    type = FunctionIC
    variable = b
    function = y
  []
  [c]
    type = FunctionIC
    variable = c
    function = z
  []
  [d]
    type = FunctionIC
    variable = d
    function = t
  []
[]

[Variables]
  [u]
  []
[]

[Materials]
  [cvf]
    type = CoupledValueFunctionMaterial
    function = test
    v = 'a b c d'
    prop_name = p
    outputs = exodus
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Transient
  dt = 0.25
  num_steps = 4
[]

[Outputs]
  exodus = true
[]
