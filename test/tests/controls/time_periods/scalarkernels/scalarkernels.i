# This tests controllability of the enable parameter of scalar kernels.
#
# There are 2 scalar variables, {u, v}, with the ODEs:
#   du/dt = 1    u(0) = 0
#   v = u        v(0) = -10
# A control switches the ODE 'v = u' to the following ODE when t >= 2:
#   dv/dt = 2
#
# 5 time steps (of size dt = 1) will be taken, and the predicted values are as follows:
#      t     u     v
# ------------------
#      0     0   -10
#      1     1     1
#      2     2     2
#      3     3     4
#      4     4     6
#      5     5     8

u_initial = 0
u_growth = 1

v_initial = -10
v_growth = 2

t_transition = 2

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
[]

[Variables]
  [./u]
    family = SCALAR
    order = FIRST
  [../]
  [./v]
    family = SCALAR
    order = FIRST
  [../]
[]

[ICs]
  [./u_ic]
    type = ScalarConstantIC
    variable = u
    value = ${u_initial}
  [../]
  [./v_ic]
    type = ScalarConstantIC
    variable = v
    value = ${v_initial}
  [../]
[]

[ScalarKernels]
  [./u_time]
    type = ODETimeDerivative
    variable = u
  [../]
  [./u_src]
    type = ParsedODEKernel
    variable = u
    expression = '-${u_growth}'
  [../]

  [./v_time]
    type = ODETimeDerivative
    variable = v
    enable = false
  [../]
  [./v_src]
    type = ParsedODEKernel
    variable = v
    expression = '-${v_growth}'
    enable = false
  [../]
  [./v_constraint]
    type = ParsedODEKernel
    variable = v
    coupled_variables = 'u'
    expression = 'v - u'
  [../]
[]

[Controls]
  [./time_period_control]
    type = TimePeriod
    end_time = ${t_transition}
    enable_objects = 'ScalarKernel::v_constraint'
    disable_objects = 'ScalarKernel::v_time ScalarKernel::v_src'
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
[]

[Executioner]
  type = Transient
  scheme = implicit-euler
  dt = 1
  num_steps = 5
  abort_on_solve_fail = true

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
[]

[Outputs]
  csv = true
[]
