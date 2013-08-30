[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = -5
  xmax = 5
  ymin = -1
  nx = 5
  elem_type = EDGE
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
    type = ParsedFunction
    value = t*x+1
  [../]
[]

[Kernels]
  [./ffn]
    type = UserForcingFunction
    variable = u
    function = forcing_fn
  [../]
  [./diffusion]
    type = Diffusion
    variable = u
  [../]
  [./timeDer]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./all]
    type = DirichletBC
    variable = u
    boundary = '0 1'
    value = 0
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

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  start_time = 0.0
  num_steps = 5
  dt = 0.1
[]

[Output]
  file_base = out_el_time_deriv_1d
  perf_log = true
  postprocessor_csv = true
[]
