[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
  nz = 4
  uniform_refine = 2
  # This test currently diffs when run in parallel with ParallelMesh enabled,
  # most likely due to the fact that CONSTANT MONOMIALS are currently not written
  # out correctly in this case.  For more information, see #2122.
  distribution = serial
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
  output_initial = true
  exodus = true
  print_linear_residuals = true
  perf_log = true
[]

