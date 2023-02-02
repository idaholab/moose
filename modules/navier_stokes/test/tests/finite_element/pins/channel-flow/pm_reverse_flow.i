# This test case tests the porous-medium flow when flow reversal happens

[GlobalParams]
  gravity = '0 0 0'

  order = FIRST
  family = LAGRANGE

  u = vel_x
  v = vel_y
  pressure = p
  temperature = T
  porosity = porosity
  eos = eos
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 0.1
  nx = 10
  ny = 4
  elem_type = QUAD4
[]

[FluidProperties]
  [eos]
    type = SimpleFluidProperties
    density0 = 100              # kg/m^3
    thermal_expansion = 0       # K^{-1}
    cp =  100
    viscosity = 0.1             # Pa-s, Re=rho*u*L/mu = 100*1*0.1/0.1 = 100
    thermal_conductivity = 0.1
  []
[]

[Functions]
  [v_in]
    type = PiecewiseLinear
    x = '0   5    10  1e5'
    y = '1   0    -1  -1'
  []
  [T_in]
    type = PiecewiseLinear
    x = '0    1e5'
    y = '630  630'
  []
[]

[Variables]
  # velocity
  [vel_x]
    initial_condition = 1
  []
  [vel_y]
    initial_condition = 0
  []
  [p]
    initial_condition = 1e5
  []
  [T]
    scaling = 1e-3
    initial_condition = 630
  []
[]

[AuxVariables]
  [rho]
    initial_condition = 100
  []
  [porosity]
    initial_condition = 0.4
  []
  [vol_heat]
    initial_condition = 1e6
  []
[]

[Materials]
  [mat]
    type = PINSFEMaterial
    alpha = 1000
    beta = 100
  []
[]


[Kernels]
  [mass_time]
    type = PINSFEFluidPressureTimeDerivative
    variable = p
  []
  [mass_space]
    type = INSFEFluidMassKernel
    variable = p
  []

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

  [temperature_time]
    type = PINSFEFluidTemperatureTimeDerivative
    variable = T
  [../]
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
    boundary = 'left'
    v_fn = v_in
  []
  # Outlet
  [./pressure_out]
    type = DirichletBC
    variable = p
    boundary = 'right'
    value = 1e5
  [../]

  # BCs for x-momentum equation
  # Inlet
  [vx_in]
    type = FunctionDirichletBC
    variable = vel_x
    boundary = 'left'
    function = v_in
  []
  # Outlet (no BC is needed)

  # BCs for y-momentum equation
  # Both Inlet and Outlet, and Top and Bottom
  [vy]
    type = DirichletBC
    variable = vel_y
    boundary = 'left right bottom top'
    value = 0
  []

  # BCs for energy equation
  [T_in]
    type = INSFEFluidEnergyDirichletBC
    variable = T
    boundary = 'left'
    out_norm = '-1 0 0'
    T_fn = 630
  []
  [T_out]
    type = INSFEFluidEnergyDirichletBC
    variable = T
    boundary = 'right'
    out_norm = '1 0 0'
    T_fn = 630
  []
[]

[Preconditioning]
  [SMP_PJFNK]
    type = SMP
    full = true
    solve_type = 'PJFNK'
  []
[]

[Postprocessors]
  [p_in]
    type = SideAverageValue
    variable = p
    boundary = left
  []
  [p_out]
    type = SideAverageValue
    variable = p
    boundary = right
  []
  [T_in]
    type = SideAverageValue
    variable = T
    boundary = left
  []
  [T_out]
    type = SideAverageValue
    variable = T
    boundary = right
  []
[]

[Executioner]
  type = Transient

  dt = 0.5
  dtmin = 1.e-3

  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'lu 100'

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8
  nl_max_its = 20

  l_tol = 1e-5
  l_max_its = 100

  start_time = 0.0
  end_time = 10
  num_steps = 20
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
