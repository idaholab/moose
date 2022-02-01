[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[Functions]
  [./left]
    type = PiecewiseConstant
    xy_data = '2.0 1
               5.5 2
               7.0 3
               8.0 4'
    direction = left
    scale_factor = 2
  [../]
  [./right]
    type = PiecewiseConstant
    x = '2.0 5.5 7.0 8.0'
    y = '1   2   3   4'
    direction = right
    scale_factor = 2
  [../]
  [./left_inclusive]
    type = PiecewiseConstant
    x = '2.0 5.5 7.0 8.0'
    y = '1   2   3   4'
    direction = left_inclusive
    scale_factor = 2
  [../]
  [./right_inclusive]
    type = PiecewiseConstant
    x = '2.0 5.5 7.0 8.0'
    y = '1   2   3   4'
    direction = right_inclusive
    scale_factor = 2
  [../]
[]

[Postprocessors]
  [left]
    type = FunctionValuePostprocessor
    function = left
    execute_on = 'TIMESTEP_END INITIAL'
  []
  [right]
    type = FunctionValuePostprocessor
    function = right
    execute_on = 'TIMESTEP_END INITIAL'
  []
  [left_inclusive]
    type = FunctionValuePostprocessor
    function = left_inclusive
    execute_on = 'TIMESTEP_END INITIAL'
  []
  [right_inclusive]
    type = FunctionValuePostprocessor
    function = right_inclusive
    execute_on = 'TIMESTEP_END INITIAL'
  []
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 10
[]

[Outputs]
  csv = true
[]
