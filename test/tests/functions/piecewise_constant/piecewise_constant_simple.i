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
    xy_data = '2.5 1
               5.0 2
               7.0 3
               8.5 4'
    direction = left
    scale_factor = 2
  [../]
  [./right]
    type = PiecewiseConstant
    x = '2.5 5.0 7.0 8.5'
    y = '1   2   3   4'
    direction = right
    scale_factor = 2
  [../]
  [./centered]
    type = PiecewiseConstant
    x = '2.5 5.0 7.0 8.5'
    y = '1   2   3   4'
    direction = centered
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
  [centered]
    type = FunctionValuePostprocessor
    function = centered
    execute_on = 'TIMESTEP_END INITIAL'
  []
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 14
[]

[Outputs]
  csv = true
[]
