[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 5
[]

[Problem]
  solve = false
[]

[Functions]
  [piecewise_linear]
    type = PiecewiseLinear
    axis = x
    x = '1 2 3 4'
    y = '4 6 10 7'
  []
[]

[VectorPostprocessors]
  [function_vpp]
    type = LineFunctionSampler
    functions = 'piecewise_linear'
    start_point = '0 0 0'
    end_point = '5 0 0'
    num_points = 6
    sort_by = x
    execute_on = 'INITIAL'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  file_base = 'no_extrap'
  execute_on = 'INITIAL'
[]
