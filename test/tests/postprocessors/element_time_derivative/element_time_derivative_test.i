[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 10
  ny = 10
  elem_type = QUAD4
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = ConstantIC
      value = 0
    [../]
  [../]
[]

[Functions]
  [./forcing_fn]
    # dudt = 3*t^2*(x^2 + y^2)
    type = ParsedFunction
    value = 3*t*t*((x*x)+(y*y))-(4*t*t*t)
  [../]
  [./forcing_fn2]
    # dudt = 3*t^2*(x^2 + y^2)
    type = ParsedFunction
    value = t*x*y
  [../]
  [./exact_fn]
    type = ParsedFunction
    value = t*t*t*((x*x)+(y*y))
  [../]
[]

[Kernels]
  [./ie]
    type = TimeDerivative
    variable = u
  [../]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./ffn]
    type = UserForcingFunction
    variable = u
    function = forcing_fn2
  [../]
[]

[BCs]
  active = 'all'
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
  [../]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]
[]

[Postprocessors]
  [./elementAvgTimeDerivative]
    type = ElementAverageTimeDerivative
    variable = u
  [../]
  [./elementAvgValue]
    type = ElementAverageValue
    variable = u
  [../]
[]

[Executioner]
  type = Transient
  scheme = implicit-euler
  petsc_options = -snes_mf_operator
  start_time = 0.0
  num_steps = 5
  dt = 0.1
[]

[Output]
  file_base = out_elm_time_deriv
  perf_log = true
  postprocessor_csv = true
[]
