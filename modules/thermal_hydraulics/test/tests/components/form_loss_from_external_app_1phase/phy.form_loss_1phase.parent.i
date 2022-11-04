# This tests a form loss transfer using the MultiApp system.  A dummy heat
# conduction problem is solved, then the form loss evaluated and transferred
# to the slave side of the solve, slave side solves and master continues solving

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmax = 2
  nx = 10
[]

[Functions]
  [left_bc_fn]
    type = PiecewiseLinear
    x = '0   1'
    y = '300 310'
  []

  [K_prime_fn]
    type = ParsedFunction
    expression = 't*(2-x)*x'
  []
[]

[AuxVariables]
  [K_prime]
  []
[]

[AuxKernels]
  [K_prime_ak]
    type = FunctionAux
    variable = K_prime
    function = K_prime_fn
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

[Executioner]
  type = Transient
  dt = 0.5
  end_time = 5
  nl_abs_tol = 1e-10
  abort_on_solve_fail = true
[]

[MultiApps]
  [slave]
    type = TransientMultiApp
    app_type = ThermalHydraulicsApp
    input_files = phy.form_loss_1phase.slave.i
    execute_on = 'timestep_end'
  []
[]

[Transfers]
  [K_to_s]
    type = MultiAppNearestNodeTransfer
    to_multi_app = slave
    source_variable = K_prime
    variable = K_prime
  []
[]
