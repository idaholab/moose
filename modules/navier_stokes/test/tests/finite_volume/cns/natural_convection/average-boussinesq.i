hot_temp=400
cold_temp=273.15
p_initial=1.01e5
T_initial=${cold_temp}
k=25.68e-3
mu=18.23e-6

[GlobalParams]
  fp = fp
  gravity = '0.00 -9.81 0.00'
  two_term_boundary_expansion = true
  limiter = central_difference
[]

[Problem]
  fv_bcs_integrity_check = false
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1e-2
    ymin = 0
    ymax = 1e-2
    nx = 32
    ny = 32
  []
[]

[Variables]
  [pressure]
    type = MooseVariableFVReal
    initial_condition = ${p_initial}
    scaling = 1e1
  []
  [sup_rho_u]
    type = MooseVariableFVReal
    initial_condition = 1e-15
    scaling = 1e3
  []
  [sup_rho_v]
    type = MooseVariableFVReal
    initial_condition = 1e-15
    scaling = 1e3
  []
  [T_fluid]
    type = MooseVariableFVReal
    initial_condition = ${T_initial}
    scaling = 1e-1
  []
  [lambda]
    family = SCALAR
    order = FIRST
  []
[]


[AuxVariables]
  [vel_x]
    type = MooseVariableFVReal
  []
  [vel_y]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [vel_x]
    type = ADMaterialRealAux
    variable = vel_x
    property = vel_x
    execute_on = 'timestep_end'
  []
  [vel_y]
    type = ADMaterialRealAux
    variable = vel_y
    property = vel_y
    execute_on = 'timestep_end'
  []
[]

[FVKernels]
  [mass_advection]
    type = PCNSFVKT
    variable = pressure
    eqn = "mass"
  []
  [mean_zero_pressure]
    type = FVScalarLagrangeMultiplier
    variable = pressure
    lambda = lambda
    phi0 = ${p_initial}
  []

  [momentum_advection]
    type = PCNSFVKT
    variable = sup_rho_u
    eqn = "momentum"
    momentum_component = 'x'
  []
  [eps_grad]
    type = PNSFVPGradEpsilon
    variable = sup_rho_u
    momentum_component = 'x'
    epsilon_function = '1'
  []
  [x_viscous]
    type = FVOrthogonalDiffusion
    variable = sup_rho_u
    coeff = ${mu}
    diffusing_quantity = 'vel_x'
  []
  [x_momentum_gravity_source]
    type = PNSFVMomentumGravity
    variable = sup_rho_u
    momentum_component = 'x'
  []

  [momentum_advection_y]
    type = PCNSFVKT
    variable = sup_rho_v
    eqn = "momentum"
    momentum_component = 'y'
  []
  [eps_grad_y]
    type = PNSFVPGradEpsilon
    variable = sup_rho_v
    momentum_component = 'y'
    epsilon_function = '1'
  []
  [y_viscous]
    type = FVOrthogonalDiffusion
    variable = sup_rho_v
    coeff = ${mu}
    diffusing_quantity = 'vel_y'
  []
  [y_momentum_gravity_source]
    type = PNSFVMomentumGravity
    variable = sup_rho_v
    momentum_component = 'y'
  []

  [energy_advection]
    type = PCNSFVKT
    variable = T_fluid
    eqn = "energy"
  []
  [fluid_energy_conduction]
    type = FVOrthogonalDiffusion
    variable = T_fluid
    coeff = ${k}
    diffusing_quantity = 'T_fluid'
  []
[]

[FVBCs]
  [pressure_x_walls]
    type = PCNSFVImplicitMomentumPressureBC
    momentum_component = 'x'
    boundary = 'left right top bottom'
    variable = sup_rho_u
  []
  [pressure_y_walls]
    type = PCNSFVImplicitMomentumPressureBC
    momentum_component = 'y'
    boundary = 'left right top bottom'
    variable = sup_rho_v
  []

  [shear_x_walls]
    type = FVOrthogonalBoundaryDiffusion
    function = 0
    variable = sup_rho_u
    diffusing_quantity = 'vel_x'
    coeff = ${mu}
    boundary = 'left right top bottom'
  []
  [shear_y_walls]
    type = FVOrthogonalBoundaryDiffusion
    function = 0
    variable = sup_rho_v
    diffusing_quantity = 'vel_y'
    coeff = ${mu}
    boundary = 'left right top bottom'
  []

  [hot_wall]
    type = FVOrthogonalBoundaryDiffusion
    function = ${hot_temp}
    variable = T_fluid
    diffusing_quantity = 'T_fluid'
    coeff = ${k}
    boundary = 'left'
  []
  [cold_wall]
    type = FVOrthogonalBoundaryDiffusion
    function = ${cold_temp}
    variable = T_fluid
    diffusing_quantity = 'T_fluid'
    coeff = ${k}
    boundary = 'right'
  []

  # Help gradient reconstruction
  [T_fluid_hot]
    type = FVDirichletBC
    variable = T_fluid
    value = ${hot_temp}
    boundary = 'left'
  []
  [T_fluid_cold]
    type = FVDirichletBC
    variable = T_fluid
    value = ${cold_temp}
    boundary = 'right'
  []
  [sup_mom_x_walls]
    type = FVDirichletBC
    variable = sup_rho_u
    value = 0
    boundary = 'left right top bottom'
  []
  [sup_mom_y_walls]
    type = FVDirichletBC
    variable = sup_rho_v
    value = 0
    boundary = 'left right top bottom'
  []
[]

[Modules]
  [FluidProperties]
    [fp]
      type = IdealGasFluidProperties
    []
  []
[]

[Materials]
  [var_mat]
    type = PorousMixedVarMaterial
    pressure = pressure
    T_fluid = T_fluid
    superficial_rhou = sup_rho_u
    superficial_rhov = sup_rho_v
    fp = fp
    porosity = porosity
  []
  [porosity]
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '1'
  []
  [functor_rho]
    type = RhoFromPTFunctorMaterial
    fp = fp
    temperature = T_fluid
    pressure = pressure
  []
[]

[Executioner]
  solve_type = NEWTON
  line_search = 'bt'
  type = Steady
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       strumpack'
[]

[Outputs]
  exodus = true
  checkpoint = true
[]

[Debug]
  show_var_residual_norms = true
[]
