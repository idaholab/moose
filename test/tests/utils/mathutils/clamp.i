[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[Functions]
  [./clamp]
    type = ClampTestFunction
  [../]
  [./exact]
    type = PiecewiseLinear
    x = '0   0.2 0.8 1.0'
    y = '0.2 0.2 0.8 0.8'
    axis = x
  [../]
[]

[VectorPostprocessors]
  [./functions]
    type = LineFunctionSampler
    functions = 'clamp exact'
    start_point = '0 0 0'
    end_point = '1 0 0'
    num_points = 10
    sort_by = x
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
