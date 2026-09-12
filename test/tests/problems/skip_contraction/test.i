[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 1
    dx = 1.0
    ix = 5
  []
  uniform_refine = 1
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []

  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Adaptivity]
  marker = uniform
  steps = 1

  [Markers/uniform]
    type = UniformMarker
    mark = coarsen
  []
[]

[Problem]
  type = SkipContractionTestProblem
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  nl_abs_tol = '1e-8'
[]

[Postprocessors]
  # Number of elements can only increase if contraction is skipped
  [n_elem]
    type = NumElements
    elem_filter = total
  []
[]

[Outputs]
  csv = true
[]
