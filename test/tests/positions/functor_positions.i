[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dx = 1
    dim = 1
  []
[]

[Positions]
  [functors]
    type = FunctorPositions
    positions_functors = '0.1 0 0.3
                          f1 f2 f1'
  []
[]

[Functions]
  [f1]
    type = PiecewiseConstant
    x = '0 0.5 1'
    y = '1 2 3'
  []
  [f2]
    type = PiecewiseLinear
    x = '0 0.5 1'
    y = '1 2 3'
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
