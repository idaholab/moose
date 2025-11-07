outlet_p = 1e5
initial_T = 300
R_pipe = 0.1
A_pipe = ${fparse pi * R_pipe^2}
A_coupled = ${A_pipe}
V_junction = ${fparse 4/3 * pi * R_pipe^3}

[GlobalParams]
  gravity_vector = '0 0 0'
  scaling_factor_1phase = '1 1 1e-5'

  initial_T = ${initial_T}
  initial_p = ${outlet_p}
  initial_vel = 0
  initial_vel_x = 0
  initial_vel_y = 0
  initial_vel_z = 0
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
  [outlet]
    type = Outlet1Phase
    input = 'pipe:in'
    p = ${outlet_p}
  []
  [pipe]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1.0
    n_elems = 5
    A = ${A_pipe}
    fp = fp
    closures = simple_closures
    f = 0
  []
  [junction]
    type = VolumeJunction1Phase
    connections = 'pipe:out'
    position = '1.0 0 0'
    volume = ${V_junction}
  []
  [junction_flux]
    type = VolumeJunctionCoupledFlux1Phase
    volume_junction = junction
    A_coupled = ${A_coupled}
    pp_suffix = test
    multi_app = child
    normal_from_junction = '1 0 0'
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[MultiApps]
  [child]
    type = TransientMultiApp
    app_type = ThermalHydraulicsApp
    input_files = child.i
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2
  end_time = 0.5
  dt = 0.1

  solve_type = NEWTON
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 10

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
  csv = true
[]
