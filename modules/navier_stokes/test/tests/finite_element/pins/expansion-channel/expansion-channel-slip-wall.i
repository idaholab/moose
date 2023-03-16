# This is an example showing the conservative form with combined velocity inlet condition,
# pressure outlet condition and slip wall boundary condition.

[GlobalParams]
  gravity = '0 -9.8 0'

  order = FIRST
  family = LAGRANGE

  u = vel_x
  v = vel_y
  pressure = p
  temperature = T
  porosity = porosity
  eos = eos

  conservative_form = true
  p_int_by_parts = true
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = expansion-channel.e
  []
  [add_corners]
    type = ExtraNodesetGenerator
    input = file
    new_boundary = 'corners'
    coord = '-0.05 -0.5 0; 0.05 -0.5 0; -0.1 0.5 0; 0.1 0.5 0'
  []
[]

[NodalNormals]
  # boundaries 3 (left) and 4 (right) are walls
  boundary = '3 4'
  corner_boundary = 'corners'
[]

[FluidProperties]
  [eos]
    type = SimpleFluidProperties
    density0 = 100              # kg/m^3
    thermal_expansion = 0.001   # K^{-1}
    cp =  100
    viscosity = 0.1             # Pa-s, Re=rho*u*L/mu = 100*1*0.1/0.1 = 100
    thermal_conductivity = 72
  []
[]


[Variables]
  # velocities
  [vel_x]
    scaling = 1e-1
    initial_condition = 0
  []
  [vel_y]
    scaling = 1e-2
    initial_condition = 1
  []

  # Pressure
  [p]
    initial_condition = 1.01e5
  []
  # Temperature
  [T]
    scaling = 1e-4
    initial_condition = 630
  []
[]

[AuxVariables]
  [rho]
    initial_condition = 77.0
  []
  [porosity]
    initial_condition = 0.6
  []
  [vol_heat]
    initial_condition = 1e3
  []
[]

[Materials]
  [mat]
    type = PINSFEMaterial
    alpha = 1e3
    beta = 100
  []
[]


[Kernels]
  # mass balance (continuity) equation
  [mass_time]
    type = PINSFEFluidPressureTimeDerivative
    variable = p
  []
  [mass_space]
    type = INSFEFluidMassKernel
    variable = p
  []

  # momentum equations for x- and y- velocities
  [x_momentum_time]
    type = PINSFEFluidVelocityTimeDerivative
    variable = vel_x
  []
  [x_momentum_space]
    type = INSFEFluidMomentumKernel
    variable = vel_x
    component = 0
  []

  [y_momentum_time]
    type = PINSFEFluidVelocityTimeDerivative
    variable = vel_y
  []
  [y_momentum_space]
    type = INSFEFluidMomentumKernel
    variable = vel_y
    component = 1
  []

  # fluid energy equation
  [temperature_time]
    type = PINSFEFluidTemperatureTimeDerivative
    variable = T
  []
  [temperature_space]
    type = INSFEFluidEnergyKernel
    variable = T
    power_density = vol_heat
  []
[]

[AuxKernels]
  [rho_aux]
    type = FluidDensityAux
    variable = rho
    p = p
    T = T
    fp = eos
  []
[]

[BCs]
  # BCs for mass equation
  # Inlet
  [mass_inlet]
    type = INSFEFluidMassBC
    variable = p
    boundary = '1'
  []
  # Outlet
  [mass_out]
    type = INSFEFluidMassBC
    variable = p
    boundary = '2'
  []

  # BCs for x-momentum equation
  # Inlet
  [vx_in]
    type = INSFEFluidMomentumBC
    variable = vel_x
    boundary = '1'
    component   = 0
    #p_fn = 1.05e5
    v_fn = 1
  []
  # Outlet
  [vx_out]
    type = INSFEFluidMomentumBC
    variable = vel_x
    boundary = '2'
    component   = 0
    p_fn = 1e5
  []
  # Walls (left and right walls)
  [vx_wall]
    type = INSFEFluidWallMomentumBC
    variable = vel_x
    boundary = '3 4'
    component = 0
  []

  # BCs for y-momentum equation
  # Inlet
  [vy_in]
    type = INSFEFluidMomentumBC
    variable = vel_y
    boundary = '1'
    component   = 1
    v_fn = 1
  []
  # Outlet
  [vy_out]
    type = INSFEFluidMomentumBC
    variable = vel_y
    boundary = '2'
    component   = 1
    p_fn = 1e5
  []
  # Walls (left and right walls)
  [vy_wall]
    type = INSFEFluidWallMomentumBC
    variable = vel_y
    boundary = '3 4'
    component = 1
  []

  # Special slip-wall BCs for both x- and y- velocities
  [slipwall]
    type = INSFEMomentumFreeSlipBC
    boundary = '3 4'
    variable = vel_x
    u = vel_x
    v = vel_y
  []

  # BCs for fluid energy equation
  # Inlet
  [T_in]
    type = INSFEFluidEnergyBC
    variable = T
    boundary = '1'
    T_fn = 630
  []
  # Outlet
  [T_out]
    type = INSFEFluidEnergyBC
    variable = T
    boundary = '2'
    T_fn = 630
  []
[]

[Preconditioning]
  [SMP_PJFNK]
    type = FDP
    full = true
    solve_type = 'PJFNK'
  []
[]

[Executioner]
  type = Transient

  dt = 0.2
  dtmin = 1.e-6
  [TimeStepper]
    type = IterationAdaptiveDT
    growth_factor = 1.25
    optimal_iterations = 15
    linear_iteration_ratio = 100
    dt = 0.1

    cutback_factor = 0.5
    cutback_factor_at_failure = 0.5
  []
  dtmax = 25

  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'lu 100'

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8
  nl_max_its = 12

  l_tol = 1e-5
  l_max_its = 100

  start_time = 0.0
  end_time = 500
  num_steps = 2
[]

[Outputs]
  perf_graph = true
  print_linear_residuals = false
  interval = 1
  execute_on = 'initial timestep_end'
  [console]
    type = Console
    output_linear = false
  []
  [out]
    type = Exodus
    use_displaced = false
  []
[]
