[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  ymin = 0
  xmax = 1
  ymax = 1
  nx = 10
  ny = 10
[]

[Functions]
  [./v_fn]
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

[AuxVariables]
  [./v]
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
  [./ufn]
    type = BodyForce
    variable = u
    function = ffn
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = 'left right top bottom'
    function = v_fn
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.1
  solve_type = 'PJFNK'

  nl_abs_tol = 1e-10
  fixed_point_max_its = 2
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

[MultiApps]
  [./sub_app]
    app_type = MooseTestApp
    type = TransientMultiApp
    input_files = 'function_dt_sub.i'
    execute_on = timestep_end
    positions = '0 -1 0'
  [../]
[]

[Transfers]
  [./from_sub]
    type = MultiAppNearestNodeTransfer
    from_multi_app = sub_app
    source_variable = u
    variable = v
  [../]
[]
