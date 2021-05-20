p_initial=1.01e5
T=273.15
# u refers to the superficial velocity
u_in=1
user_limiter='min_mod'

[GlobalParams]
  fp = fp
  two_term_boundary_expansion = true
  limiter = ${user_limiter}
[]

[Mesh]
  [cartesian]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 18
    nx = 45
    ymin = 0
    ymax = 1
    ny = 3
  []
  [pt5]
    input = cartesian
    type = SubdomainBoundingBoxGenerator
    bottom_left = '3 0 0'
    top_right = '7 1 0'
    block_id = 1
  []
  [pt25]
    input = pt5
    type = SubdomainBoundingBoxGenerator
    bottom_left = '7 0 0'
    top_right = '11 1 0'
    block_id = 2
  []
  [pt5_again]
    input = pt25
    type = SubdomainBoundingBoxGenerator
    bottom_left = '11 0 0'
    top_right = '15 1 0'
    block_id = 3
  []
  [one]
    input = pt5_again
    type = SubdomainBoundingBoxGenerator
    bottom_left = '15 0 0'
    top_right = '18 1 0'
    block_id = 4
  []
[]

[Modules]
  [FluidProperties]
    [fp]
      type = IdealGasFluidProperties
    []
  []
[]

[Problem]
  kernel_coverage_check = false
  fv_bcs_integrity_check = false
[]

[Variables]
  [pressure]
    type = MooseVariableFVReal
    initial_condition = ${p_initial}
  []
  [sup_vel_x]
    type = MooseVariableFVReal
    initial_condition = 1e-15
    scaling = 1e-2
  []
  [sup_vel_y]
    type = MooseVariableFVReal
    initial_condition = 1e-15
    scaling = 1e-2
  []
  [T_fluid]
    type = MooseVariableFVReal
    initial_condition = ${T}
    scaling = 1e-5
  []
[]

[AuxVariables]
  [vel_x]
    type = MooseVariableFVReal
  []
  [sup_mom_x]
    type = MooseVariableFVReal
  []
  [rho]
    type = MooseVariableFVReal
  []
  [eps]
    type = MooseVariableFVReal
  []
[]

[ICs]
  [eps]
    type = FunctionIC
    variable = eps
    function = eps
  []
[]

[AuxKernels]
  [vel_x]
    type = ADMaterialRealAux
    variable = vel_x
    property = vel_x
    execute_on = 'timestep_end'
  []
  [sup_mom_x]
    type = ADMaterialRealAux
    variable = sup_mom_x
    property = superficial_rhou
    execute_on = 'timestep_end'
  []
  [rho]
    type = ADMaterialRealAux
    variable = rho
    property = rho
    execute_on = 'timestep_end'
  []
[]

[FVKernels]
  [mass_time]
    type = FVMatPropTimeKernel
    mat_prop_time_derivative = 'dsuperficial_rho_dt'
    variable = pressure
  []
  [mass_advection]
    type = PCNSFVKT
    variable = pressure
    eqn = "mass"
  []

  [momentum_time]
    type = FVMatPropTimeKernel
    mat_prop_time_derivative = 'dsuperficial_rhou_dt'
    variable = sup_vel_x
  []
  [momentum_advection]
    type = PCNSFVKT
    variable = sup_vel_x
    eqn = "momentum"
    momentum_component = 'x'
  []
  [eps_grad]
    type = PNSFVPGradEpsilon
    variable = sup_vel_x
    momentum_component = 'x'
    epsilon_var = 'eps'
  []
  [drag]
    type = PNSFVMomentumFriction
    variable = sup_vel_x
    momentum_component = 'x'
    Darcy_name = 'cl'
    momentum_name = superficial_rhou
  []

  [momentum_time_y]
    type = FVMatPropTimeKernel
    mat_prop_time_derivative = 'dsuperficial_rhov_dt'
    variable = sup_vel_y
  []
  [momentum_advection_y]
    type = PCNSFVKT
    variable = sup_vel_y
    eqn = "momentum"
    momentum_component = 'y'
  []
  [eps_grad_y]
    type = PNSFVPGradEpsilon
    variable = sup_vel_y
    momentum_component = 'y'
    epsilon_var = 'eps'
  []
  [drag_y]
    type = PNSFVMomentumFriction
    variable = sup_vel_y
    momentum_component = 'y'
    Darcy_name = 'cl'
    momentum_name = superficial_rhov
  []

  [energy_time]
    type = FVMatPropTimeKernel
    mat_prop_time_derivative = 'dsuperficial_rho_et_dt'
    variable = T_fluid
  []
  [energy_advection]
    type = PCNSFVKT
    variable = T_fluid
    eqn = "energy"
  []
[]

[FVBCs]
  [rho_left]
    type = PCNSFVKTBC
    boundary = 'left'
    variable = pressure
    superficial_velocity_function = 'ud_in'
    T_fluid_function = ${T}
    eqn = 'mass'
  []
  [rhou_left]
    type = PCNSFVKTBC
    boundary = 'left'
    variable = sup_vel_x
    superficial_velocity_function = 'ud_in'
    T_fluid_function = ${T}
    eqn = 'momentum'
    momentum_component = 'x'
  []
  [rhov_left]
    type = PCNSFVKTBC
    boundary = 'left'
    variable = sup_vel_y
    superficial_velocity_function = 'ud_in'
    T_fluid_function = ${T}
    eqn = 'momentum'
    momentum_component = 'y'
  []
  [rho_et_left]
    type = PCNSFVKTBC
    boundary = 'left'
    variable = T_fluid
    superficial_velocity_function = 'ud_in'
    T_fluid_function = ${T}
    eqn = 'energy'
  []
  [rho_right]
    type = PCNSFVKTBC
    boundary = 'right'
    variable = pressure
    pressure_function = ${p_initial}
    eqn = 'mass'
  []
  [rhou_right]
    type = PCNSFVKTBC
    boundary = 'right'
    variable = sup_vel_x
    pressure_function = ${p_initial}
    eqn = 'momentum'
    momentum_component = 'x'
  []
  [rhov_right]
    type = PCNSFVKTBC
    boundary = 'right'
    variable = sup_vel_y
    pressure_function = ${p_initial}
    eqn = 'momentum'
    momentum_component = 'y'
  []
  [rho_et_right]
    type = PCNSFVKTBC
    boundary = 'right'
    variable = T_fluid
    pressure_function = ${p_initial}
    eqn = 'energy'
  []

  [wall_pressure_x]
    type = PCNSFVImplicitMomentumPressureBC
    momentum_component = 'x'
    boundary = 'top bottom'
    variable = sup_vel_x
  []
  [wall_pressure_y]
    type = PCNSFVImplicitMomentumPressureBC
    momentum_component = 'y'
    boundary = 'top bottom'
    variable = sup_vel_y
  []

  # Use these to help create more accurate cell centered gradients for cells adjacent to boundaries
  [T_left]
    type = FVDirichletBC
    variable = T_fluid
    value = ${T}
    boundary = 'left'
  []
  [sup_vel_left]
    type = FVDirichletBC
    variable = sup_vel_x
    value = ${u_in}
    boundary = 'left'
  []
  [sup_vel_x_walls]
    type = FVDirichletBC
    variable = sup_vel_x
    value = 0
    boundary = 'top bottom'
  []
  [sup_vel_y_left_and_walls]
    type = FVDirichletBC
    variable = sup_vel_y
    value = 0
    boundary = 'left top bottom'
  []
  [p_right]
    type = FVDirichletBC
    variable = pressure
    value = ${p_initial}
    boundary = 'right'
  []
  [eps]
    type = FVFunctionDirichletBC
    variable = eps
    function = eps
    boundary = 'left right top bottom'
  []
[]

[Functions]
  [ud_in]
    type = ParsedVectorFunction
    value_x = '${u_in}'
  []
  [eps]
    type = ParsedFunction
    value = 'if(x < 3, 1,
             if(x < 7, 0.5,
             if(x < 11, 0.25,
             if(x < 15, 0.5, 1))))'
  []
[]

[Materials]
  [var_mat]
    type = PorousPrimitiveVarMaterial
    pressure = pressure
    T_fluid = T_fluid
    superficial_vel_x = sup_vel_x
    superficial_vel_y = sup_vel_y
    fp = fp
    porosity = porosity
  []
  [zero]
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '1'
    block = 0
  []
  [one]
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '0.5'
    block = 1
  []
  [two]
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '0.25'
    block = 2
  []
  [three]
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '0.5'
    block = 3
  []
  [four]
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '1'
    block = 4
  []
  [ad_generic]
    type = ADGenericConstantVectorMaterial
    prop_names = 'cl'
    prop_values = '100 100 100'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  solve_type = NEWTON
  line_search = 'bt'

  type = Transient
  nl_max_its = 20
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 5e-5
    optimal_iterations = 6
    growth_factor = 1.2
  []
  num_steps = 10000
  end_time = 500
  nl_abs_tol = 1e-8

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       mumps'
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = 'final'
  []
  checkpoint = true
[]

[Debug]
  show_var_residual_norms = true
[]
