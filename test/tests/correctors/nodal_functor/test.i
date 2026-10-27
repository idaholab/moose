[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 1
    dx = 2
    ix = 2
  []
[]

[Variables]
  [u]
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Functions]
  [setter]
    type = ParsedFunction
    expression = '2 + x + 10 * t'
  []
[]

[Correctors]
  [corr]
    type = FunctorNodalCorrector
    variables_to_correct = u
    functors = 'setter'
    execution_order_group = -1
  []
[]

[Executioner]
  type = Transient
  num_steps = 4
[]

[Postprocessors]
  [min_u]
    type = NodalExtremeValue
    value_type = 'min'
    variable = 'u'
  []
  [max_u]
    type = NodalExtremeValue
    value_type = 'max'
    variable = 'u'
  []
[]

[Outputs]
  csv = true
[]
