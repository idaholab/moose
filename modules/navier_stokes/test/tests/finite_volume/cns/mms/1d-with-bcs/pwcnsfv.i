rho='rho'
advected_interp_method='upwind'
velocity_interp_method='rc'
gamma=1.4
R=8.3145
molar_mass=29.0e-3
R_specific=${fparse R/molar_mass}
cp=${fparse gamma*R_specific/(gamma-1)}

[GlobalParams]
  two_term_boundary_expansion = true
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = PINSFVRhieChowInterpolator
    u = sup_vel_x
    pressure = pressure
    porosity = porosity
  []
[]

[Mesh]
  [cartesian]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = .1
    xmax = .6
    nx = 2
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
    type = INSFVPressureVariable
  []
  [sup_vel_x]
    type = PINSFVSuperficialVelocityVariable
  []
[]

[AuxVariables]
  [porosity]
    type = MooseVariableFVReal
  []
  [T_fluid]
    type = INSFVEnergyVariable
  []
[]

[ICs]
  [pressure]
    type = FunctionIC
    variable = pressure
    function = 'exact_p'
  []
  [sup_vel_x]
    type = FunctionIC
    variable = sup_vel_x
    function = 'exact_sup_vel_x'
  []
  [T_fluid]
    type = FunctionIC
    variable = T_fluid
    function = 'exact_T'
  []
  [eps]
    type = FunctionIC
    variable = porosity
    function = 'eps'
  []
[]

[FVKernels]
  [mass_advection]
    type = PINSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
  []
  [mass_fn]
    type = FVBodyForce
    variable = pressure
    function = 'forcing_rho'
  []

  [u_advection]
    type = PINSFVMomentumAdvection
    variable = sup_vel_x
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    porosity = porosity
    momentum_component = 'x'
  []
  [u_pressure]
    type = PINSFVMomentumPressureFlux
    variable = sup_vel_x
    pressure = pressure
    porosity = porosity
    momentum_component = 'x'
    force_boundary_execution = false
  []
  [momentum_fn]
    type = INSFVBodyForce
    variable = sup_vel_x
    functor = 'forcing_rho_ud'
    momentum_component = 'x'
  []
[]

[FVBCs]
  [mass]
    variable = pressure
    type = PINSFVFunctorBC
    boundary = 'left right'
    superficial_vel_x = sup_vel_x
    pressure = pressure
    eqn = 'mass'
    porosity = porosity
  []
  [momentum]
    variable = sup_vel_x
    type = PINSFVFunctorBC
    boundary = 'left right'
    superficial_vel_x = sup_vel_x
    pressure = pressure
    eqn = 'momentum'
    momentum_component = 'x'
    porosity = porosity
  []

  # help gradient reconstruction *and* create Dirichlet values for use in PINSFVFunctorBC
  [pressure_right]
    type = FVFunctionDirichletBC
    variable = pressure
    function = exact_p
    boundary = 'right'
  []
  [sup_vel_x_left]
    type = FVFunctionDirichletBC
    variable = sup_vel_x
    function = exact_sup_vel_x
    boundary = 'left'
  []
  [T_fluid_left]
    type = FVFunctionDirichletBC
    variable = T_fluid
    function = exact_T
    boundary = 'left'
  []
[]

[Materials]
  [const_functor]
    type = ADGenericFunctorMaterial
    prop_names = 'cp'
    prop_values = '${cp}'
  []
  [rho]
    type = RhoFromPTFunctorMaterial
    fp = fp
    temperature = T_fluid
    pressure = pressure
  []
  [ins_fv]
    type = INSFVEnthalpyMaterial
    temperature = T_fluid
    rho = ${rho}
  []
[]

[Functions]
  [forcing_rho]
    type = ParsedFunction
    expression = '-3.45300378856215*sin(1.1*x)'
  []
  [forcing_rho_ud]
    type = ParsedFunction
    expression = '-0.9*(10.6975765229419*cos(1.2*x)/cos(x) - 0.697576522941849*cos(1.1*x)^2/cos(x)^2)*sin(x) + 0.9*(10.6975765229419*sin(x)*cos(1.2*x)/cos(x)^2 - 1.3951530458837*sin(x)*cos(1.1*x)^2/cos(x)^3 + 1.53466835047207*sin(1.1*x)*cos(1.1*x)/cos(x)^2 - 12.8370918275302*sin(1.2*x)/cos(x))*cos(x) + 3.13909435323832*sin(x)*cos(1.1*x)^2/cos(x)^2 - 6.9060075771243*sin(1.1*x)*cos(1.1*x)/cos(x)'
  []
  [exact_T]
    type = ParsedFunction
    expression = '0.0106975765229418*cos(1.2*x)/cos(x) - 0.000697576522941848*cos(1.1*x)^2/cos(x)^2'
  []
  [exact_p]
    type = ParsedFunction
    expression = '3.48788261470924*(3.06706896551724*cos(1.2*x)/cos(x) - 0.2*cos(1.1*x)^2/cos(x)^2)*cos(x)'
  []
  [exact_sup_vel_x]
    type = ParsedFunction
    expression = '0.9*cos(1.1*x)/cos(x)'
  []
  [eps]
    type = ParsedFunction
    expression = '0.9'
  []
[]

[Executioner]
  solve_type = NEWTON
  type = Transient
  num_steps = 1
  dtmin = 1
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_max_its = 50
  line_search = bt
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-12
[]

[Outputs]
  exodus = true
  csv = true
[]

[Debug]
  show_var_residual_norms = true
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2pressure]
    type = ElementL2FunctorError
    approximate = pressure
    exact = exact_p
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2sup_vel_x]
    approximate = sup_vel_x
    exact = exact_sup_vel_x
    type = ElementL2FunctorError
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]
