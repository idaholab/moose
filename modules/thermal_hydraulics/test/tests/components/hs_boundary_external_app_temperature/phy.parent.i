# This tests a transfer of temperature values computed by master app and used by a slave app
# as a heat structure boundary condition

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmax = 1
  nx = 10
[]

[Functions]
  [T_bc_fn]
    type = ParsedFunction
    expression = '300+t*x*10'
  []
  [T_ffn]
    type = ParsedFunction
    expression = 'x*10'
  []
[]

[Variables]
  [T]
  []
[]

[ICs]
  [T_ic]
    type = ConstantIC
    variable = T
    value = 300
  []
[]

[Kernels]
  [td]
    type = ADTimeDerivative
    variable = T
  []

  [diff]
    type = ADDiffusion
    variable = T
  []

  [ffn]
    type = BodyForce
    variable = T
    function = T_ffn
  []
[]

[BCs]
  [left]
    type = FunctionDirichletBC
    variable = T
    boundary = 'left right'
    function = T_bc_fn
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 2
  nl_abs_tol = 1e-10
  abort_on_solve_fail = true
  solve_type = NEWTON
[]

[MultiApps]
  [thm]
    type = TransientMultiApp
    app_type = ThermalHydraulicsApp
    input_files = phy.slave.i
    execute_on = 'initial timestep_end'
  []
[]

[Transfers]
  [T_to_thm]
    type = MultiAppNearestNodeTransfer
    to_multi_app = thm
    source_variable = T
    variable = T_ext
    target_boundary = 'hs:outer'
  []
[]

[Outputs]
  exodus = true
  show = 'T'
[]
