[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  nz = 10
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./solution]
    type = ParsedFunction
    value = (exp(x)-1)/(exp(1)-1)
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./conv]
    type = Convection
    variable = u
    velocity = '1 0 0'
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
[]

[Adaptivity]
  [./Indicators]
    [./ai]
      function = solution
      type = AnalyticalIndicator
      field_name = error
      variable = u
    [../]
  [../]
  [./Markers]
    [./efm]
      type = ErrorFractionMarker
      field_name = marker
      coarsen = 0.1
      refine = 0.3
      indicator_field = error
    [../]
  [../]
[]

[Output]
  output_initial = true
  exodus = true
  print_linear_residuals = true
  perf_log = true
[]

