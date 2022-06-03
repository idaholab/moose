# This tests a temperature transfer using the MultiApp system.  Simple heat
# conduction problem is solved, then the layered average is computed and
# transferred into the slave side of the solve

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmax = 1
  nx = 10
  parallel_type = replicated
[]

[Functions]
  [left_bc_fn]
    type = PiecewiseLinear
    x = '0   1'
    y = '300 310'
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
    type = TimeDerivative
    variable = T
  []

  [diff]
    type = Diffusion
    variable = T
  []
[]

[BCs]
  [left]
    type = FunctionDirichletBC
    variable = T
    boundary = left
    function = left_bc_fn
  []
[]

[UserObjects]
  [T_avg_uo]
    type = LayeredAverage
    variable = T
    direction = x
    num_layers = 5
  []
[]

[Executioner]
  type = Transient
  dt = 0.5
  end_time = 5
  nl_abs_tol = 1e-10
  abort_on_solve_fail = true
[]

[MultiApps]
  [thm]
    type = TransientMultiApp
    app_type = ThermalHydraulicsApp
    input_files = phy.T_wall_transfer_elem_3eqn.slave.i
    execute_on = TIMESTEP_END
  []
[]

[Transfers]
  [T_to_slave]
    type = MultiAppUserObjectTransfer
    to_multi_app = thm
    user_object = T_avg_uo
    variable = T_wall
  []
[]

[Outputs]
  exodus = true
[]
