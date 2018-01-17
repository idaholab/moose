[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 5
  ny = 5
  nz = 5
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  zmin = -1
  zmax = 1
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diffusion]
    type = Diffusion
    variable = u
  [../]
[]

[Functions]
  [./series]
    type = FunctionSeries
    series_type = Cartesian
    r = Legendre
    orders = '0'
    physical_bounds = '-1 1'
    coefficients = '1.0'
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]

