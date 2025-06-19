[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    ix = 4
    iy = 4
    dx = 1
    dy = 1
    dim = 2
  []
[]

[Positions]
  [all_elems]
    type = ElementCentroidPositions
    outputs = 'none'
  []
  [functors_downselection]
    type = ParsedDownSelectionPositions
    input_positions = 'all_elems'
    expression = '(t > 1) & (x > 0.4) & (sym_var_y > 0.7) & (f1 > 2)'
    functor_symbols = 'sym_var_y f1'
    functor_names = 'var_y       f1'
  []
[]

[AuxVariables]
  [var_y]
    [InitialCondition]
      type = FunctionIC
      function = 'y'
    []
  []
[]

[Functions]
  [f1]
    type = PiecewiseConstant
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
