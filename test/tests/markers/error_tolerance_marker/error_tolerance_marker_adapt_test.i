[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
  nz = 4
  uniform_refine = 2
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

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Adaptivity]
  steps = 1
  marker = marker
  [./Indicators]
    [./error]
      type = AnalyticalIndicator
      variable = u
      function = solution
    [../]
  [../]
  [./Markers]
    [./marker]
      type = ErrorToleranceMarker
      coarsen = 3e-10
      indicator = error
      refine = 7e-10
    [../]
  [../]
[]

[Output]
  linear_residuals = true
  output_initial = true
  exodus = true
  perf_log = true
[]

