[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 1
  nx = 10
[]

[Functions]
  [./u_fn]
    type = ParsedFunction
    expression = t*x
  [../]
  [./ffn]
    type = ParsedFunction
    expression = x
  [../]

  [./dts]
    type = PiecewiseLinear
    x = '0.1  10'
    y = '0.1  10'
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./td]
    type = TimeDerivative
    variable = u
  [../]
  [./fn]
    type = BodyForce
    variable = u
    function = ffn
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
    type = FunctionDirichletBC
    variable = u
    boundary = right
    function = u_fn
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.1
  solve_type = 'PJFNK'

  nl_abs_tol = 1e-10
  start_time = 0
  num_steps = 3
  [./TimeStepper]
    type = FunctionDT
    function = dts
  [../]
[]

[Outputs]
  exodus = true
[]
