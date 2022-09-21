nelem = 50
friction_factor = 1e4

area = 0.176752
mfr_final = 1.0
p_out = 7e6
T_in = 300
ramp_time = 5.0

[GlobalParams]
  gravity_vector = '0 0 0'
  initial_T = ${T_in}
  initial_p = ${p_out}
  initial_vel = 0
  closures = closures
  rdg_slope_reconstruction = full
  scaling_factor_1phase = '1 1 1e-5'
[]

[FluidProperties]
  [h2]
    type = IdealGasFluidProperties
    gamma = 1.3066
    molar_mass = 2.016e-3
    k = 0.437
    mu = 3e-5
  []
[]

[Closures]
  [closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [bc_inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'ch_1:in'
    m_dot = 0 # This value is controlled by 'mfr_ctrl'
    T = ${T_in}
  []
  [ch_1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 0.5
    n_elems = ${nelem}
    A = ${area}
    f = ${friction_factor}
    fp = h2
  []
  [junction]
    type = JunctionOneToOne1Phase
    connections = 'ch_1:out ch_2:in'
  []
  [ch_2]
    type = FlowChannel1Phase
    position = '0.5 0 0'
    orientation = '1 0 0'
    length = 0.5
    n_elems = ${nelem}
    A = ${area}
    f = ${friction_factor}
    fp = h2
  []
  [bc_outlet]
    type = Outlet1Phase
    input = 'ch_2:out'
    p = ${p_out}
  []
[]

[Functions]
  [mfr_fn]
    type = PiecewiseLinear
    x = '0 ${ramp_time}'
    y = '0 ${mfr_final}'
  []
[]

[ControlLogic]
  [mfr_cntrl]
    type = TimeFunctionComponentControl
    component = bc_inlet
    parameter = m_dot
    function = mfr_fn
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[VectorPostprocessors]
  [pressure_vpp]
    type = ADSampler1DReal
    block = 'ch_1 ch_2'
    property = 'p'
    sort_by = x
    execute_on = 'FINAL'
  []
[]

[Executioner]
  type = Transient

  scheme = bdf2
  start_time = 0
  end_time = 50
  dt = 1

  steady_state_detection = true
  steady_state_start_time = ${ramp_time}

  petsc_options_iname = '-pc_type'
  petsc_options_value = ' lu     '
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  nl_max_its = 15
  l_tol = 1e-4
  l_max_its = 10
[]

[Outputs]
  [csv]
    type = CSV
    create_final_symlink = true
    execute_on = 'FINAL'
  []
[]
