# This test case tests the porous-medium flow driven by pressure gradient
#
# At the steady state, eps * grad_p = alpha * u + beta * u^2
# With eps = 0.4, L = 1, grad_p = 1e3/1 = 1e3, alpha = 0, beta = 1000
# u = (eps * grad_p) / beta = 0.4 * 1e3 / 1000 = 0.4 m/s
# This can be verified by check the vel_x at the steady state

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

  conservative_form = false
  p_int_by_parts = true
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
  [./eos]
    type = SimpleFluidProperties
    density0 = 100              # kg/m^3
    thermal_expansion = 0       # K^{-1}
    cp =  100
    viscosity = 0.1             # Pa-s, Re=rho*u*L/mu = 100*1*0.1/0.1 = 100
  [../]
[]

[Variables]
  # velocity
  [vel_x]
    initial_condition = 1
  []
  [vel_y]
    initial_condition = 0
  []
  # Pressure
  [p]
    initial_condition = 1e5
  []
[]

[AuxVariables]
  [rho]
    initial_condition = 100
  []
  # Temperature
  [T]
    initial_condition = 630
  []
  [porosity]
    initial_condition = 0.4
  []
[]

[Materials]
  [mat]
    type = PINSFEMaterial
    alpha = 0
    beta = 1000
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
  []
  # Outlet
  [mass_outlet]
    type = INSFEFluidMassBC
    variable = p
    boundary = 'right'
  []

  # BCs for x-momentum equation
  # Inlet
  [vx_in]
    type = INSFEFluidMomentumBC
    variable = vel_x
    component = 0
    boundary = 'left'
    p_fn = 1.01e5
  []
  # Outlet
  [vx_out]
    type = INSFEFluidMomentumBC
    variable = vel_x
    component = 0
    boundary = 'right'
    p_fn = 1e5
  []

  # BCs for y-momentum equation
  # Both Inlet and Outlet, and Top and Bottom
  [vy]
    type = DirichletBC
    variable = vel_y
    boundary = 'left right bottom top'
    value = 0
  []
[]

[Postprocessors]
  [v_in]
    type = SideAverageValue
    variable = vel_x
    boundary = 'left'
  []
[]

[Preconditioning]
  [SMP_PJFNK]
    type = SMP
    full = true
    solve_type = 'PJFNK'
  []
[]

[Executioner]
  type = Transient

  dt = 5
  dtmin = 1.e-3

  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'lu 100'

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8
  nl_max_its = 20

  l_tol = 1e-5
  l_max_its = 100

  start_time = 0.0
  end_time = 50
  num_steps = 5
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
