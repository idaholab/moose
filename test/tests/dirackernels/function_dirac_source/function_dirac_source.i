[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 5
  ny = 5
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[DiracKernels]
  [./point_source]
    type = FunctionDiracSource
    variable = u
    function = switch_off
    point = '0.1 0.2 0.0'
  [../]
[]

[Functions]
  [./switch_off]
    type = ParsedFunction
    expression = 'if(t < 1.0001, 1, 0)'
  [../]
[]

[BCs]
  [./external]
    type = NeumannBC
    variable = u
    boundary = '0 1 2 3'
    value = 0
  [../]
[]

[Postprocessors]
  [./total_internal_energy]
    type = ElementIntegralVariablePostprocessor
    variable = u
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 1
  l_tol = 1e-03
[]

[Outputs]
  exodus = true
[]
