# Reference for ad_array_robin.i with one standard variable per component
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 6
  ny = 6
[]

[Variables]
  [u0]
  []
  [u1]
  []
[]

[Kernels]
  [diff0]
    type = ADDiffusion
    variable = u0
  []
  [diff1]
    type = ADDiffusion
    variable = u1
  []
[]

[BCs]
  [left0]
    type = ADDirichletBC
    variable = u0
    boundary = left
    value = 1
  []
  [left1]
    type = ADDirichletBC
    variable = u1
    boundary = left
    value = 2
  []
  [robin0]
    # alpha u0 contribution
    type = ADRobinBC
    variable = u0
    boundary = right
    coefficient = 2
  []
  [flux0]
    type = ADNeumannBC
    variable = u0
    boundary = right
    value = 0.5
  []
  [flux1]
    type = ADNeumannBC
    variable = u1
    boundary = right
    value = -1
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  # this input and ad_array_robin.i share a gold file, so both converge far below the CSVDiff
  # relative tolerance of 5.5e-6 for their results to agree regardless of the solver path
  nl_rel_tol = 1e-12
[]

[Postprocessors]
  [int0]
    type = ElementIntegralVariablePostprocessor
    variable = u0
  []
  [int1]
    type = ElementIntegralVariablePostprocessor
    variable = u1
  []
[]

[Outputs]
  file_base = robin_out
  csv = true
[]
