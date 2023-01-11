p_initial=1.01e5
T=273.15
u_in=10
eps=1
superficial_vel_in=${fparse u_in * eps}

[GlobalParams]
  fp = fp
  limiter = 'vanLeer'
  two_term_boundary_expansion = true
[]

[Mesh]
  [cartesian]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = 10
    nx = 100
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
  [superficial_vel_x]
    type = MooseVariableFVReal
    initial_condition = ${superficial_vel_in}
  []
  [temperature]
    type = MooseVariableFVReal
    initial_condition = ${T}
  []
[]

[AuxVariables]
  [rho]
    type = MooseVariableFVReal
  []
  [superficial_rhou]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [rho]
    type = ADMaterialRealAux
    variable = rho
    property = rho
    execute_on = 'timestep_end'
  []
  [superficial_rhou]
    type = ADMaterialRealAux
    variable = superficial_rhou
    property = superficial_rhou
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
    variable = superficial_vel_x
  []
  [momentum_advection]
    type = PCNSFVKT
    variable = superficial_vel_x
    eqn = "momentum"
    momentum_component = 'x'
  []

  [energy_time]
    type = FVMatPropTimeKernel
    mat_prop_time_derivative = 'dsuperficial_rho_et_dt'
    variable = temperature
  []
  [energy_advection]
    type = PCNSFVKT
    variable = temperature
    eqn = "energy"
  []
  [heat]
    type = FVBodyForce
    variable = temperature
    value = 1e6
  []
[]

[FVBCs]
  [rho_left]
    type = PCNSFVStrongBC
    boundary = 'left'
    variable = pressure
    superficial_velocity = 'superficial_vel_in'
    T_fluid = ${T}
    eqn = 'mass'
  []
  [rhou_left]
    type = PCNSFVStrongBC
    boundary = 'left'
    variable = superficial_vel_x
    superficial_velocity = 'superficial_vel_in'
    T_fluid = ${T}
    eqn = 'momentum'
    momentum_component = 'x'
  []
  [rho_et_left]
    type = PCNSFVStrongBC
    boundary = 'left'
    variable = temperature
    superficial_velocity = 'superficial_vel_in'
    T_fluid = ${T}
    eqn = 'energy'
  []
  [rho_right]
    type = PCNSFVStrongBC
    boundary = 'right'
    variable = pressure
    pressure = ${p_initial}
    eqn = 'mass'
  []
  [rhou_right]
    type = PCNSFVStrongBC
    boundary = 'right'
    variable = superficial_vel_x
    pressure = ${p_initial}
    eqn = 'momentum'
    momentum_component = 'x'
  []
  [rho_et_right]
    type = PCNSFVStrongBC
    boundary = 'right'
    variable = temperature
    pressure = ${p_initial}
    eqn = 'energy'
  []

  # Use these to help create more accurate cell centered gradients for cells adjacent to boundaries
  [T_left]
    type = FVDirichletBC
    variable = temperature
    value = ${T}
    boundary = 'left'
  []
  [sup_vel_left]
    type = FVDirichletBC
    variable = superficial_vel_x
    value = ${superficial_vel_in}
    boundary = 'left'
  []
  [p_right]
    type = FVDirichletBC
    variable = pressure
    value = ${p_initial}
    boundary = 'right'
  []
[]

[Functions]
  [superficial_vel_in]
    type = ParsedVectorFunction
    expression_x = '${superficial_vel_in}'
  []
[]

[Materials]
  [var_mat]
    type = PorousPrimitiveVarMaterial
    pressure = pressure
    T_fluid = temperature
    superficial_vel_x = superficial_vel_x
    fp = fp
    porosity = porosity
  []
  [fluid_only]
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '${eps}'
  []
[]

[Executioner]
  solve_type = NEWTON
  type = Transient
  nl_max_its = 20
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 5e-5
    optimal_iterations = 10
  []
  steady_state_detection = false
  steady_state_tolerance = 1e-12
  abort_on_solve_fail = false
  end_time = 100
  nl_abs_tol = 1e-8
  dtmin = 5e-5
  automatic_scaling = true
  compute_scaling_once = false
  verbose = true

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type -pc_factor_shift_type -snes_linesearch_minlambda'
  petsc_options_value = 'lu       mumps                      NONZERO               1e-3 '
[]

[Outputs]
  [exo]
    type = Exodus
    execute_on = 'final'
  []
  [dof]
    type = DOFMap
    execute_on = 'initial'
  []
  checkpoint = true
[]

[Debug]
  show_var_residual_norms = true
[]
