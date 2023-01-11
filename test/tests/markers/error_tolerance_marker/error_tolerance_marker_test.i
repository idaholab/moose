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
    expression = (exp(x)-1)/(exp(1)-1)
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

  solve_type = 'PJFNK'
[]

[Adaptivity]
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
      coarsen = 4e-9
      indicator = error
      refine = 1e-8
    [../]
  [../]
[]

[Outputs]
  exodus = true
[]
