# Test 8 from the following reference:
#
#   F. Daude, P. Galon. A Finite-Volume approach for compressible single- and
#   two-phase flows in flexible pipelines with fluid-structure interaction.
#   Journal of Computational Physics 362 (2018) 375-408.

L1 = 10
L2 = 3
L3 = 5

xJ = ${L1}
x_p1 = ${fparse xJ - 1.05}
x_p2 = ${fparse xJ + 0.15}
x_p3 = ${fparse xJ + 0.95}

N1 = 1000
N2 = 300
N3 = 500

D1 = 0.35682482
D2 = 0.19544100
D3 = 0.35682482

A1 = ${fparse 0.25 * pi * D1^2}
A2 = ${fparse 0.25 * pi * D2^2}
A3 = ${fparse 0.25 * pi * D3^2}
AJ = ${fparse A1 + A2 + A3}
RJ = ${fparse sqrt(AJ / (4 * pi))} # A = 4 pi R^2
VJ = ${fparse 4/3 * pi * RJ^3}

y2 = 1
y3 = -1

gamma = 2.23
p_inf = 1e9 # denoted by "pi" in reference
q = 0
cv = 2500 # arbitrary value; not given in reference

CFL = 0.8
t_end = 0.01

p_out = 80e5

initial_p = ${p_out}
initial_T = 327.1864956 # reference has rho = 1001.89 kg/m^3
initial_vel1 = 1
initial_vel2 = 0.769
initial_vel3 = 0.769

[GlobalParams]
  gravity_vector = '0 0 0'

  initial_T = ${initial_T}
  initial_p = ${initial_p}

  fp = fp
  closures = closures
  f = 0

  rdg_slope_reconstruction = none
  scaling_factor_1phase = '1 1 1e-5'
[]

[FluidProperties]
  [fp]
    type = StiffenedGasFluidProperties
    gamma = ${gamma}
    p_inf = ${p_inf}
    q = ${q}
    cv = ${cv}
  []
[]

[Closures]
  [closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [pipe1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = ${L1}
    n_elems = ${N1}
    A = ${A1}
    initial_vel = ${initial_vel1}
  []
  [pipe2]
    type = FlowChannel1Phase
    position = '${xJ} ${y2} 0'
    orientation = '1 0 0'
    length = ${L2}
    n_elems = ${N2}
    A = ${A2}
    initial_vel = ${initial_vel2}
  []
  [pipe3]
    type = FlowChannel1Phase
    position = '${xJ} ${y3} 0'
    orientation = '1 0 0'
    length = ${L3}
    n_elems = ${N3}
    A = ${A3}
    initial_vel = ${initial_vel3}
  []

  [junction]
    type = VolumeJunction1Phase
    connections = 'pipe1:out pipe2:in pipe3:in'
    position = '${xJ} 0 0'
    volume = ${VJ}
    initial_vel_x = ${initial_vel2} # ?
    initial_vel_y = 0
    initial_vel_z = 0
    scaling_factor_rhoEV = 1e-5
    apply_velocity_scaling = true
  []

  [outlet1]
    type = Outlet1Phase
    input = 'pipe1:in'
    p = ${p_out}
  []
  [outlet2]
    type = Outlet1Phase
    input = 'pipe2:out'
    p = ${p_out}
  []
  [wall3]
    type = SolidWall1Phase
    input = 'pipe3:out'
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [dt_cfl]
    type = ADCFLTimeStepSize
    CFL = ${CFL}
    vel_names = 'vel'
    c_names = 'c'
  []
  [p1]
    type = PointValue
    variable = p
    point = '${x_p1} 0 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [p2]
    type = PointValue
    variable = p
    point = '${x_p2} ${y2} 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [p3]
    type = PointValue
    variable = p
    point = '${x_p3} ${y3} 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient

  start_time = 0
  end_time = ${t_end}
  [TimeStepper]
    type = PostprocessorDT
    postprocessor = dt_cfl
  []
  [TimeIntegrator]
    type = ActuallyExplicitEuler
  []

  solve_type = LINEAR

  petsc_options_iname = '-pc_type'
  petsc_options_value = ' lu     '

  l_tol = 1e-4
  l_max_its = 10
[]

[Times]
  [output_times]
    type = TimeIntervalTimes
    time_interval = 1e-4
  []
[]

[Outputs]
  file_base = 'three_pipe_shock'
  [csv]
    type = CSV
    show = 'p1 p2 p3'
    sync_only = true
    sync_times_object = output_times
  []
[]
