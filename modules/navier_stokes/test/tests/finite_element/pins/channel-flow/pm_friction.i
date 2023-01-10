# This test case tests the porous-medium flow pressure drop due to friction (both viscous and inertia effect)
#
# At the steady state, eps * grad_p = alpha * u + beta * u^2
# With eps = 0.4, L = 1, u = 1, alpha = 1000, beta = 100
# dp = (1000 + 100) / 0.4 = 2,750
# This can be verified by check the p_in - p_out

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
  [./eos]
    type = SimpleFluidProperties
    density0 = 100              # kg/m^3
    thermal_expansion = 0       # K^{-1}
    cp =  100
    viscosity = 0.1             # Pa-s, Re=rho*u*L/mu = 100*1*0.1/0.1 = 100
  [../]
[]

[Functions]
  [v_in]
    type = PiecewiseLinear
    x = '0   1e5'
    y = '1     1'
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
    type = GenericPorousMediumMaterial
    alpha = 1000
    beta = 100
  []
[]


[Kernels]
  [mass_time]
    type = PMFluidPressureTimeDerivative
    variable = p
  []
  [mass_space]
    type = MDFluidMassKernel
    variable = p
  []

  [x_momentum_time]
    type = PMFluidVelocityTimeDerivative
    variable = vel_x
  []
  [x_momentum_space]
    type = MDFluidMomentumKernel
    variable = vel_x
    component = 0
  []

  [y_momentum_time]
    type = PMFluidVelocityTimeDerivative
    variable = vel_y
  []
  [y_momentum_space]
    type = MDFluidMomentumKernel
    variable = vel_y
    component = 1
  []
[]

[AuxKernels]
  [rho_aux]
    type = NSDensityAux
    variable = rho
  []
[]

[BCs]
  # BCs for mass equation
  # Inlet
  [mass_inlet]
    type = MDFluidMassBC
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
[]

[Executioner]
  type = Transient

  dt = 0.1
  dtmin = 1.e-3

  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'lu 100'

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8
  nl_max_its = 20

  l_tol = 1e-5
  l_max_its = 100

  start_time = 0.0
  end_time = 0.5
  num_steps = 10
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
