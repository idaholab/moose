# This problem models passive transport variables going into a Y-junction:
#
#           / pipeO1
#  pipeI   /
#    -----*
#          \
#           \ pipeO2
#
# Area = 0.1
#
# Inlet values:
#   passiveA_inlet = 0.4
#   passiveB_inlet = 0.2
#
# Initially the 2 outlet pipes have 5 times the concentration of each passive as
# in the inlet pipe. Then after holding the inlet conditions steady for some time,
# the outlet concentrations should match the inlet concentrations.
#
# Theoretical steady outlet values:
#   passiveA_times_area = Area * passiveA_inlet = 0.04
#   passiveA_times_area = Area * passiveB_inlet = 0.02

A = 0.1
V_junction = 0.01
vel_inlet = 1.0
p_outlet = 1e5
rho_inlet = 1.162633159 # rho @ (1e5 Pa, 300 K)

mdot_inlet = ${fparse rho_inlet * vel_inlet * A}
T_inlet = 300
passiveA_inlet = 0.4
passiveB_inlet = 0.2

p_initial = ${p_outlet}
T_initial = ${T_inlet}
passiveA_initial_pipeI = ${passiveA_inlet}
passiveB_initial_pipeI = ${passiveB_inlet}
passiveA_initial_pipeO = ${fparse 5*passiveA_inlet}
passiveB_initial_pipeO = ${fparse 5*passiveB_inlet}

n_elems = 20
L_pipe = 0.5

[GlobalParams]
  gravity_vector = '0 0 0'
  length = ${L_pipe}
  n_elems = ${n_elems}
  A = ${A}

  initial_T = ${T_initial}
  initial_p = ${p_initial}
  initial_vel = 0
  initial_vel_x = 0
  initial_vel_y = 0
  initial_vel_z = 0

  passives_names = 'passiveA passiveB'

  scaling_factor_1phase = '1 1 1e-5'

  fp = fp
  closures = simple_closures
  f = 0
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'pipeI:in'
    m_dot = ${mdot_inlet}
    T = ${T_inlet}
    passives = '${passiveA_inlet} ${passiveB_inlet}'
  []
  [pipeI]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    initial_passives = '${passiveA_initial_pipeI} ${passiveB_initial_pipeI}'
  []
  [junction]
    type = VolumeJunction1Phase
    position = '${L_pipe} 0 0'
    connections = 'pipeI:out pipeO1:in pipeO2:in'
    volume = ${V_junction}
    initial_passives = '${passiveA_initial_pipeI} ${passiveB_initial_pipeI}'
  []
  [pipeO1]
    type = FlowChannel1Phase
    position = '${L_pipe} 0 0'
    orientation = '1 1 1'
    initial_passives = '${passiveA_initial_pipeO} ${passiveB_initial_pipeO}'
  []
  [pipeO2]
    type = FlowChannel1Phase
    position = '${L_pipe} 0 0'
    orientation = '1 -1 -1'
    initial_passives = '${passiveA_initial_pipeO} ${passiveB_initial_pipeO}'
  []
  [outlet1]
    type = Outlet1Phase
    input = 'pipeO1:out'
    p = ${p_outlet}
  []
  [outlet2]
    type = Outlet1Phase
    input = 'pipeO2:out'
    p = ${p_outlet}
  []
[]

[Postprocessors]
  [passiveA_times_area_out1]
    type = SideAverageValue
    variable = passiveA_times_area
    boundary = 'pipeO1:out'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [passiveA_times_area_out2]
    type = SideAverageValue
    variable = passiveA_times_area
    boundary = 'pipeO2:out'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [passiveB_times_area_out1]
    type = SideAverageValue
    variable = passiveB_times_area
    boundary = 'pipeO1:out'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [passiveB_times_area_out2]
    type = SideAverageValue
    variable = passiveB_times_area
    boundary = 'pipeO2:out'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2

  start_time = 0
  dt = 0.2
  num_steps = 20

  solve_type = NEWTON
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8
  nl_max_its = 30

  l_tol = 1e-3
  l_max_its = 10
[]

[Outputs]
  csv = true
[]
