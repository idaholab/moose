p_initial=1.01e5
T=273.15
# u refers to the superficial velocity
u_in=1
rho_in=1.30524
sup_mom_y_in=${fparse u_in * rho_in}
user_limiter='upwind'
friction_coeff=10

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
    xmax = 1
    nx = 3
    ymin = 0
    ymax = 18
    ny = 90
  []
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
  []
[]

[Problem]
  fv_bcs_integrity_check = false
[]

[Variables]
  [pressure]
    type = MooseVariableFVReal
    initial_condition = ${p_initial}
  []
  [sup_mom_x]
    type = MooseVariableFVReal
    initial_condition = 1e-15
    scaling = 1e-2
  []
  [sup_mom_y]
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
  [vel_y]
    type = MooseVariableFVReal
  []
  [rho]
    type = MooseVariableFVReal
  []
  [eps]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [vel_y]
    type = ADMaterialRealAux
    variable = vel_y
    property = vel_y
    execute_on = 'timestep_end'
  []
  [rho]
    type = ADMaterialRealAux
    variable = rho
    property = rho
    execute_on = 'timestep_end'
  []
  [eps]
    type = MaterialRealAux
    variable = eps
    property = porosity
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
    variable = sup_mom_x
  []
  [momentum_advection]
    type = PCNSFVKT
    variable = sup_mom_x
    eqn = "momentum"
    momentum_component = 'x'
  []
  [eps_grad]
    type = PNSFVPGradEpsilon
    variable = sup_mom_x
    momentum_component = 'x'
    epsilon_function = 'eps'
  []
  [drag]
    type = PCNSFVMomentumFriction
    variable = sup_mom_x
    momentum_component = 'x'
    Darcy_name = 'cl'
    momentum_name = superficial_rhou
  []

  [momentum_time_y]
    type = FVMatPropTimeKernel
    mat_prop_time_derivative = 'dsuperficial_rhov_dt'
    variable = sup_mom_y
  []
  [momentum_advection_y]
    type = PCNSFVKT
    variable = sup_mom_y
    eqn = "momentum"
    momentum_component = 'y'
  []
  [eps_grad_y]
    type = PNSFVPGradEpsilon
    variable = sup_mom_y
    momentum_component = 'y'
    epsilon_function = 'eps'
  []
  [drag_y]
    type = PCNSFVMomentumFriction
    variable = sup_mom_y
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
  [rho_bottom]
    type = PCNSFVStrongBC
    boundary = 'bottom'
    variable = pressure
    superficial_velocity = 'ud_in'
    T_fluid = ${T}
    eqn = 'mass'
    velocity_function_includes_rho = true
  []
  [rhou_bottom]
    type = PCNSFVStrongBC
    boundary = 'bottom'
    variable = sup_mom_x
    superficial_velocity = 'ud_in'
    T_fluid = ${T}
    eqn = 'momentum'
    momentum_component = 'x'
    velocity_function_includes_rho = true
  []
  [rhov_bottom]
    type = PCNSFVStrongBC
    boundary = 'bottom'
    variable = sup_mom_y
    superficial_velocity = 'ud_in'
    T_fluid = ${T}
    eqn = 'momentum'
    momentum_component = 'y'
    velocity_function_includes_rho = true
  []
  [rho_et_bottom]
    type = PCNSFVStrongBC
    boundary = 'bottom'
    variable = T_fluid
    superficial_velocity = 'ud_in'
    T_fluid = ${T}
    eqn = 'energy'
    velocity_function_includes_rho = true
  []
  [rho_top]
    type = PCNSFVStrongBC
    boundary = 'top'
    variable = pressure
    pressure = ${p_initial}
    eqn = 'mass'
  []
  [rhou_top]
    type = PCNSFVStrongBC
    boundary = 'top'
    variable = sup_mom_x
    pressure = ${p_initial}
    eqn = 'momentum'
    momentum_component = 'x'
  []
  [rhov_top]
    type = PCNSFVStrongBC
    boundary = 'top'
    variable = sup_mom_y
    pressure = ${p_initial}
    eqn = 'momentum'
    momentum_component = 'y'
  []
  [rho_et_top]
    type = PCNSFVStrongBC
    boundary = 'top'
    variable = T_fluid
    pressure = ${p_initial}
    eqn = 'energy'
  []

  [wall_pressure_x]
    type = PCNSFVImplicitMomentumPressureBC
    momentum_component = 'x'
    boundary = 'left right'
    variable = sup_mom_x
  []
  [wall_pressure_y]
    type = PCNSFVImplicitMomentumPressureBC
    momentum_component = 'y'
    boundary = 'left right'
    variable = sup_mom_y
  []

  # Use these to help create more accurate cell centered gradients for cells adjacent to boundaries
  [T_bottom]
    type = FVDirichletBC
    variable = T_fluid
    value = ${T}
    boundary = 'bottom'
  []
  [sup_mom_x_bottom_and_walls]
    type = FVDirichletBC
    variable = sup_mom_x
    value = 0
    boundary = 'bottom left right'
  []
  [sup_mom_y_walls]
    type = FVDirichletBC
    variable = sup_mom_y
    value = 0
    boundary = 'left right'
  []
  [sup_mom_y_bottom]
    type = FVDirichletBC
    variable = sup_mom_y
    value = ${sup_mom_y_in}
    boundary = 'bottom'
  []
  [p_top]
    type = FVDirichletBC
    variable = pressure
    value = ${p_initial}
    boundary = 'top'
  []
[]

[Functions]
  [ud_in]
    type = ParsedVectorFunction
    expression_x = '0'
    expression_y = '${sup_mom_y_in}'
  []
  [eps]
    type = ParsedFunction
    expression = 'if(y < 2.8, 1,
             if(y < 3.2, 1 - .5 / .4 * (y - 2.8),
             if(y < 6.8, .5,
             if(y < 7.2, .5 - .25 / .4 * (y - 6.8),
             if(y < 10.8, .25,
             if(y < 11.2, .25 + .25 / .4 * (y - 10.8),
             if(y < 14.8, .5,
             if(y < 15.2, .5 + .5 / .4 * (y - 14.8),
                1))))))))'
  []
[]

[Materials]
  [var_mat]
    type = PorousMixedVarMaterial
    pressure = pressure
    T_fluid = T_fluid
    superficial_rhou = sup_mom_x
    superficial_rhov = sup_mom_y
    fp = fp
    porosity = porosity
  []
  [porosity]
    type = GenericFunctionMaterial
    prop_names = 'porosity'
    prop_values = 'eps'
  []
  [ad_generic]
    type = ADGenericConstantVectorMaterial
    prop_names = 'cl'
    prop_values = '${friction_coeff} ${friction_coeff} ${friction_coeff}'
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
  nl_abs_tol = 1e-7

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
