[Mesh]
  [cmg]
    type = GeneratedMeshGenerator
    nx = 10
    ny = 10
    dim = 2
  []
[]

[Positions]
  [functors]
    type = FunctorExtremaPositions
    functor = 'f1'
    extrema_type = 'MAX_ABS'
    num_extrema = 10
  []
[]

[Functions]
  [f1]
    type = ParsedFunction
    # avoid identical values
    expression = '2 * t * (x*x + 3.00005*y + 0.25)'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  # Test recover
  num_steps = 2
[]

[Outputs]
  [out]
    type = JSON
    execute_system_information_on = none
  []
[]
