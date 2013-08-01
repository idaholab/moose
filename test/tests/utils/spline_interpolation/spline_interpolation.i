[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 4
  xmin = -1
  xmax = 3
  elem_type = EDGE2
[]

[Functions]
  [./spline_fn]
    type = SplineFunction
    x = '-1  0 3'
    y = '0.5 0 3'
  [../]
[]

[Variables]
  [./u]
    order = THIRD
    family = HERMITE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  
  [./ufn]
    type = SplineFFn
    variable = u
    function = spline_fn
  [../]
[]

[BCs]
  [./sides]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1'
    function = spline_fn
  [../]
[]

[Postprocessors]
  [./l2_err]
    type = ElementL2Error
    variable = u
    function = spline_fn
  [../]
[]

[Executioner]
  type = Steady
[]

[Output]
  exodus = true
  output_initial = true
  perf_log = true
[]
