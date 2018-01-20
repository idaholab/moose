[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = -1
  xmax = 1
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
    x = Legendre
    disc = Zernike
    orders = '0'
    physical_bounds = '-1 1'
  [../]
[]

[Executioner]
  type = Steady
[]
