# This tests a heat flux transfer using the MultiApp system.  Simple heat
# conduction problem is solved, then the heat flux is picked up by the slave
# side of the solve, slave side solves and transfers its variables back to the
# master

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmax = 1
  nx = 10
[]

[Functions]
  [sin_fn]
    type = ParsedFunction
    expression = '1000*t*sin(pi*x)'
  []
[]

[Variables]
  [T]
  []
[]

[AuxVariables]
  [q_wall]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [q_wal_ak]
    type = FunctionAux
    variable = q_wall
    function = sin_fn
    execute_on = 'initial timestep_end'
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
    type = DirichletBC
    variable = T
    boundary = 'left right'
    value = 300
  []
[]

[Executioner]
  type = Transient
  dt = 0.5
  num_steps = 2
  nl_abs_tol = 1e-10
  abort_on_solve_fail = true
[]

[MultiApps]
  [thm]
    type = TransientMultiApp
    app_type = ThermalHydraulicsApp
    input_files = phy.q_wall_transfer_3eqn.slave.i
    execute_on = 'initial timestep_end'
  []
[]

[Transfers]
  [q_to_thm]
    type = MultiAppNearestNodeTransfer
    to_multi_app = thm
    source_variable = q_wall
    variable = q_wall
  []
[]

[Outputs]
  exodus = true
  show = 'q_wall'
[]
