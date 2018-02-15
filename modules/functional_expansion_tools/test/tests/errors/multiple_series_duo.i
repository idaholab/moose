[Mesh]
  type = GeneratedMesh
  dim = 3
[]

[Functions]
  [./series]
    type = FunctionSeries
    series_type = CylindricalDuo
    orders = '0 1'
    physical_bounds = '-1.0 1.0   0.0 0.0   1'
    x = Legendre
    disc = Zernike
    y = Legendre
  [../]
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]
