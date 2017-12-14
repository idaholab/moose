[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 10
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  zmin = -1
  zmax = 1
[]

[AuxVariables]
  [./a]
  [../]
  [./a_exact]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[AuxKernels]
  [./a]
    type = FunctionAux
    variable = a
    function = 'series'
  [../]
  [./error]
    type = ElementL2ErrorFunctionAux
    variable = a_exact
    function = exact_series
    coupled_variable = a
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
    y = Legendre
    z = Legendre
    orders = '0 1 1'
    physical_bounds = '-1 1 -1 1 -1 1'
    coefficients = '1.0 2.0 3.0 4.0'
  [../]
  [./exact_series]
    type = ParsedFunction
    value = '1+2*z+3*y+4*y*z'
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
