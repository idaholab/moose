[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Functions]
  [./series]
    type = FunctionSeries
    series_type = Cartesian
    orders = '0'
    physical_bounds = '-1 1'
  [../]
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]
