# Junction of 3 pipes:
#
#   1     3
# -----*-----
#      | 2
#
# The left end of Pipe 1 is a high-pressure region, and the rest of the system
# is at a low pressure.
#
# Pipe 1 is closed, while Pipes 2 and 3 are open.

end_time = 0.07

D_pipe = 0.01
A_pipe = ${fparse 0.25 * pi * D_pipe^2}
length_pipe1_HP = 0.53
length_pipe1_LP = 3.10
length_pipe2 = 2.595
length_pipe3 = 1.725

x_junction = ${fparse length_pipe1_HP + length_pipe1_LP}

# Numbers of elements correspond to dx ~ 1/3 cm
n_elems_pipe1_HP = 159
n_elems_pipe1_LP = 930
n_elems_pipe2 = 779
n_elems_pipe3 = 518

S_junction = ${fparse 3 * A_pipe}
r_junction = ${fparse sqrt(S_junction / (4 * pi))}
V_junction = ${fparse 4/3 * pi * r_junction^3}

p_low = 1e5
p_high = 1.15e5

T_low  = 283.5690633 # at p = 1e5 Pa,    rho = 1.23 kg/m^3
T_high = 283.5690633 # at p = 1.15e5 Pa, rho = 1.4145 kg/m^3

cfl = 0.95

[GlobalParams]
  # common FlowChannel1Phase parameters
  A = ${A_pipe}
  initial_vel = 0
  fp = fp_air
  closures = closures
  f = 0
  gravity_vector = '0 0 0'
  scaling_factor_1phase = '1 1 1e-5'
[]

[FluidProperties]
  [fp_air]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 0.029
  []
[]

[Closures]
  [closures]
    type = Closures1PhaseSimple
  []
[]

[Functions]
  [initial_T_pipe1_fn]
    type = PiecewiseConstant
    axis = x
    x = '0 ${length_pipe1_HP}'
    y = '${T_high} ${T_low}'
  []
  [initial_p_pipe1_fn]
    type = PiecewiseConstant
    axis = x
    x = '0 ${length_pipe1_HP}'
    y = '${p_high} ${p_low}'
  []
[]

[Components]
  [pipe1_wall]
    type = SolidWall1Phase
    input = 'pipe1:in'
  []
  [pipe1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = '${length_pipe1_HP} ${length_pipe1_LP}'
    n_elems = '${n_elems_pipe1_HP} ${n_elems_pipe1_LP}'
    initial_p = initial_p_pipe1_fn
    initial_T = initial_T_pipe1_fn
  []

  [junction]
    type = VolumeJunction1Phase
    position = '${x_junction} 0 0'
    connections = 'pipe1:out pipe2:in pipe3:in'
    initial_p = ${p_low}
    initial_T = ${T_low}
    initial_vel_x = 0
    initial_vel_y = 0
    initial_vel_z = 0
    volume = ${V_junction}
    scaling_factor_rhoEV = 1e-5
    apply_velocity_scaling = true
  []

  [pipe2]
    type = FlowChannel1Phase
    position = '${x_junction} 0 0'
    orientation = '0 -1 0'
    length = ${length_pipe2}
    n_elems = ${n_elems_pipe2}
    initial_p = ${p_low}
    initial_T = ${T_low}
  []
  [pipe2_outlet]
    type = Outlet1Phase
    input = 'pipe2:out'
    p = ${p_low}
  []

  [pipe3]
    type = FlowChannel1Phase
    position = '${x_junction} 0 0'
    orientation = '1 0 0'
    length = ${length_pipe3}
    n_elems = ${n_elems_pipe3}
    initial_p = ${p_low}
    initial_T = ${T_low}
  []
  [pipe3_outlet]
    type = Outlet1Phase
    input = 'pipe3:out'
    p = ${p_low}
  []
[]

[Postprocessors]
  [cfl_dt]
    type = ADCFLTimeStepSize
    CFL = ${cfl}
    c_names = 'c'
    vel_names = 'vel'
  []
  [p_pipe1_048]
    type = PointValue
    variable = p
    point = '${fparse x_junction - 0.48} 0 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [p_pipe2_052]
    type = PointValue
    variable = p
    point = '${fparse x_junction} -0.52 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [p_pipe3_048]
    type = PointValue
    variable = p
    point = '${fparse x_junction + 0.48} 0 0'
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
  end_time = ${end_time}

  [TimeIntegrator]
    type = ExplicitSSPRungeKutta
    order = 1
  []

  [TimeStepper]
    type = PostprocessorDT
    postprocessor = cfl_dt
  []
  abort_on_solve_fail = true

  solve_type = LINEAR
[]

[Times]
  [output_times]
    type = TimeIntervalTimes
    time_interval = 7e-4
  []
[]

[Outputs]
  file_base = '3pipes_open'
  [csv]
    type = CSV
    show = 'p_pipe1_048 p_pipe2_052 p_pipe3_048'
    sync_only = true
    sync_times_object = output_times
  []
  [console]
    type = Console
    execute_postprocessors_on = 'NONE'
  []
[]
