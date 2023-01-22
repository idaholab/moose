# This tests a temperature and heat transfer coefficient using the MultiApp system.
# Simple heat conduction problem with heat source is solved,
# to obtain an analytical solution:
# T(x,t) = 300 + 20t(x-1)^2
# The temperature is picked up by the slave
# side of the solve, to use as ambiant temperature in a convective BC.
htc = 100

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmax = 1
  nx = 10
[]

[Functions]
  [left_bc_fn]
    type = PiecewiseLinear
    x = '0   10'
    y = '300 500'
  []
[]

[Variables]
  [T]
  []
[]

[AuxVariables]
  [htc_ext]
    initial_condition = ${htc}
  []
[]

[Functions]
  [source_term]
    type = ParsedFunction
    expression = '20 * x * x - 40 * x + 20 - 40 * t'
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

  [source]
    type = BodyForce
     function = 'source_term'
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
  [right]
    type = NeumannBC
    variable = T
    boundary = right
    value = 0
  []
[]

[Executioner]
  type = Transient
  dt = 0.5
  end_time = 10
  nl_abs_tol = 1e-10
  abort_on_solve_fail = true
[]

[MultiApps]
  [thm]
    type = TransientMultiApp
    app_type = ThermalHydraulicsApp
    input_files = plate.i
    execute_on = TIMESTEP_END
  []
[]

[Transfers]
  [T_to_slave]
    type = MultiAppNearestNodeTransfer
    to_multi_app = thm
    source_variable = T
    variable = T_ext
  []
  [htc_to_slave]
    type = MultiAppNearestNodeTransfer
    to_multi_app = thm
    source_variable = htc_ext
    variable = htc_ext
  []
[]

[Outputs]
  exodus = true
[]
