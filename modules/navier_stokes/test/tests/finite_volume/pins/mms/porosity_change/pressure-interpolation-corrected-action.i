mu=1.1
rho=1.1
darcy=1.1
forch=1.1

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 2
    ymin = -1
    ymax = 1
    nx = 2
    ny = 2
  []
[]

[AuxVariables]
  [eps_out]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [eps_out]
    type = ADFunctorElementalAux
    variable = eps_out
    functor = porosity
    execute_on = 'timestep_end'
  []
[]

[Modules]
  [NavierStokesFV]
    compressibility = 'incompressible'

    porous_medium_treatment = true
    porosity = porosity
    porosity_smoothing_layers = 2

    friction_types = 'darcy forchheimer'
    friction_coeffs = 'Darcy_coefficient Forchheimer_coefficient'
    use_friction_correction = true
    consistent_scaling = 1.0

    density = 'rho'
    dynamic_viscosity = 'mu'

    initial_velocity = '1 1 0'
    initial_pressure = 0.0

    inlet_boundaries = 'left top bottom'
    momentum_inlet_types = 'fixed-velocity fixed-velocity fixed-velocity'
    momentum_inlet_function = 'exact_u exact_v; exact_u exact_v; exact_u exact_v'

    outlet_boundaries = 'right'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = 'exact_p'

    mass_advection_interpolation = 'average'
    momentum_advection_interpolation = 'average'
  []
[]

[FVKernels]
  [mass_forcing]
    type = FVBodyForce
    variable = pressure
    function = forcing_p
  []
  [u_forcing]
    type = INSFVBodyForce
    variable = superficial_vel_x
    functor = forcing_u
    momentum_component = 'x'
    rhie_chow_user_object = 'pins_rhie_chow_interpolator'
  []
  [v_forcing]
    type = INSFVBodyForce
    variable = superficial_vel_y
    functor = forcing_v
    momentum_component = 'y'
    rhie_chow_user_object = 'pins_rhie_chow_interpolator'
  []
[]

[Materials]
  [darcy]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'Darcy_coefficient Forchheimer_coefficient'
    prop_values = '${darcy} ${darcy} ${darcy} ${forch} ${forch} ${forch}'
  []
  [constants]
    type = ADGenericFunctorMaterial
    prop_names = 'rho mu'
    prop_values = '${rho} ${mu}'
  []
[]

[Functions]
  [porosity]
    type = ParsedFunction
    expression = '.5 + .1 * sin(pi * x / 4) * cos(pi * y / 4)'
  []

  [exact_u]
    type = ParsedFunction
    expression = 'sin((1/2)*y*pi)*cos((1/2)*x*pi)'
  []
  [forcing_u]
    type = ParsedFunction
    expression = '-mu*(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)*(-1/4*pi^2*sin((1/2)*y*pi)*cos((1/2)*x*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) + 0.025*pi^2*sin((1/4)*x*pi)*sin((1/4)*y*pi)*cos((1/2)*x*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2 + 0.00625*pi^2*sin((1/4)*x*pi)*sin((1/2)*y*pi)*cos((1/2)*x*pi)*cos((1/4)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2 + 0.00125*pi^2*sin((1/4)*x*pi)^2*sin((1/4)*y*pi)^2*sin((1/2)*y*pi)*cos((1/2)*x*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^3) - mu*(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)*(-1/4*pi^2*sin((1/2)*y*pi)*cos((1/2)*x*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) + 0.00625*pi^2*sin((1/4)*x*pi)*sin((1/2)*y*pi)*cos((1/2)*x*pi)*cos((1/4)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2 + 0.025*pi^2*sin((1/2)*x*pi)*sin((1/2)*y*pi)*cos((1/4)*x*pi)*cos((1/4)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2 + 0.00125*pi^2*sin((1/2)*y*pi)*cos((1/4)*x*pi)^2*cos((1/2)*x*pi)*cos((1/4)*y*pi)^2/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^3) - 0.025*pi*mu*(-1/2*pi*sin((1/2)*x*pi)*sin((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) - 0.025*pi*sin((1/2)*y*pi)*cos((1/4)*x*pi)*cos((1/2)*x*pi)*cos((1/4)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2)*cos((1/4)*x*pi)*cos((1/4)*y*pi) + 0.025*pi*mu*((1/2)*pi*cos((1/2)*x*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) + 0.025*pi*sin((1/4)*x*pi)*sin((1/4)*y*pi)*sin((1/2)*y*pi)*cos((1/2)*x*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2)*sin((1/4)*x*pi)*sin((1/4)*y*pi) + rho*(darcy + forch)*sin((1/2)*y*pi)*cos((1/2)*x*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) - 1/2*pi*rho*sin((1/4)*x*pi)*sin((1/2)*y*pi)^2*cos((1/2)*x*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) + (1/2)*pi*rho*sin((1/4)*x*pi)*cos((1/2)*x*pi)*cos((1/2)*y*pi)^2/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) - pi*rho*sin((1/2)*x*pi)*sin((1/2)*y*pi)^2*cos((1/2)*x*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) + 0.025*pi*rho*sin((1/4)*x*pi)^2*sin((1/4)*y*pi)*sin((1/2)*y*pi)*cos((1/2)*x*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2 - 0.025*pi*rho*sin((1/2)*y*pi)^2*cos((1/4)*x*pi)*cos((1/2)*x*pi)^2*cos((1/4)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2 - 1/4*pi*(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)*sin((1/4)*x*pi)*sin((3/2)*y*pi)'
    symbol_names = 'mu rho darcy forch'
    symbol_values = '${mu} ${rho} ${darcy} ${forch}'
  []
  [exact_v]
    type = ParsedFunction
    expression = 'sin((1/4)*x*pi)*cos((1/2)*y*pi)'
  []
  [forcing_v]
    type = ParsedFunction
    expression = '-mu*(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)*(-1/4*pi^2*sin((1/4)*x*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) - 0.025*pi^2*sin((1/4)*x*pi)^2*sin((1/4)*y*pi)*sin((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2 + 0.00625*pi^2*sin((1/4)*x*pi)^2*cos((1/4)*y*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2 + 0.00125*pi^2*sin((1/4)*x*pi)^3*sin((1/4)*y*pi)^2*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^3) - mu*(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)*(-1/16*pi^2*sin((1/4)*x*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) + 0.00625*pi^2*sin((1/4)*x*pi)^2*cos((1/4)*y*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2 - 0.0125*pi^2*cos((1/4)*x*pi)^2*cos((1/4)*y*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2 + 0.00125*pi^2*sin((1/4)*x*pi)*cos((1/4)*x*pi)^2*cos((1/4)*y*pi)^2*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^3) + 0.025*pi*mu*(-1/2*pi*sin((1/4)*x*pi)*sin((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) + 0.025*pi*sin((1/4)*x*pi)^2*sin((1/4)*y*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2)*sin((1/4)*x*pi)*sin((1/4)*y*pi) - 0.025*pi*mu*((1/4)*pi*cos((1/4)*x*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) - 0.025*pi*sin((1/4)*x*pi)*cos((1/4)*x*pi)*cos((1/4)*y*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2)*cos((1/4)*x*pi)*cos((1/4)*y*pi) + rho*(darcy + forch)*sin((1/4)*x*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) - pi*rho*sin((1/4)*x*pi)^2*sin((1/2)*y*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) - 1/2*pi*rho*sin((1/4)*x*pi)*sin((1/2)*x*pi)*sin((1/2)*y*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) + (1/4)*pi*rho*sin((1/2)*y*pi)*cos((1/4)*x*pi)*cos((1/2)*x*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5) + 0.025*pi*rho*sin((1/4)*x*pi)^3*sin((1/4)*y*pi)*cos((1/2)*y*pi)^2/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2 - 0.025*pi*rho*sin((1/4)*x*pi)*sin((1/2)*y*pi)*cos((1/4)*x*pi)*cos((1/2)*x*pi)*cos((1/4)*y*pi)*cos((1/2)*y*pi)/(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)^2 + (3/2)*pi*(0.1*sin((1/4)*x*pi)*cos((1/4)*y*pi) + 0.5)*cos((1/4)*x*pi)*cos((3/2)*y*pi)'
    symbol_names = 'mu rho darcy forch'
    symbol_values = '${mu} ${rho} ${darcy} ${forch}'
  []
  [exact_p]
    type = ParsedFunction
    expression = 'sin((3/2)*y*pi)*cos((1/4)*x*pi)'
  []
  [forcing_p]
    type = ParsedFunction
    expression = '-1/2*pi*rho*sin((1/4)*x*pi)*sin((1/2)*y*pi) - 1/2*pi*rho*sin((1/2)*x*pi)*sin((1/2)*y*pi)'
    symbol_names = 'rho'
    symbol_values = '${rho}'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      100                lu           NONZERO'
  line_search = 'none'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = false
  csv = true
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2u]
    type = ElementL2FunctorError
    approximate = superficial_vel_x
    exact = exact_u
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2v]
    type = ElementL2FunctorError
    approximate = superficial_vel_y
    exact = exact_v
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2p]
    type = ElementL2FunctorError
    approximate = pressure
    exact = exact_p
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]
