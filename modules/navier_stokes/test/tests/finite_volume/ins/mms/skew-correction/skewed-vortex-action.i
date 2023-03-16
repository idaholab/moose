mu = 1.0
rho = 1.0

[Mesh]
  [gen_mesh]
    type = FileMeshGenerator
    file = skewed.msh
  []
[]

[Modules]
  [NavierStokesFV]
    compressibility = 'incompressible'

    density = 'rho'
    dynamic_viscosity = 'mu'

    wall_boundaries = 'top left right bottom'
    momentum_wall_types = 'noslip noslip noslip noslip'

    initial_velocity = '1 1 0'

    pin_pressure = true
    pinned_pressure_type = average
    pinned_pressure_value = 0

    momentum_face_interpolation = skewness-corrected
    pressure_face_interpolation = skewness-corrected

    momentum_advection_interpolation = skewness-corrected
    mass_advection_interpolation = skewness-corrected
  []
[]

[FVKernels]
  [u_forcing]
    type = INSFVBodyForce
    variable = vel_x
    functor = forcing_u
    momentum_component = 'x'
    rhie_chow_user_object = 'ins_rhie_chow_interpolator'
  []
  [v_forcing]
    type = INSFVBodyForce
    variable = vel_y
    functor = forcing_v
    momentum_component = 'y'
    rhie_chow_user_object = 'ins_rhie_chow_interpolator'
  []
[]

[Materials]
  [const]
    type = ADGenericFunctorMaterial
    prop_names = 'rho mu'
    prop_values = '${rho} ${mu}'
  []
[]

[Functions]
  [exact_u]
    type = ParsedFunction
    expression = 'x^2*(1-x)^2*(2*y-6*y^2+4*y^3)'
  []
  [exact_v]
    type = ParsedFunction
    expression = '-y^2*(1-y)^2*(2*x-6*x^2+4*x^3)'
  []
  [exact_p]
    type = ParsedFunction
    expression = 'x*(1-x)-2/12'
  []
  [forcing_u]
    type = ParsedFunction
    expression = '-4*mu/rho*(-1+2*y)*(y^2-6*x*y^2+6*x^2*y^2-y+6*x*y-6*x^2*y+3*x^2-6*x^3+3*x^4)+1-2*x+4*x^3'
            '*y^2*(2*y^2-2*y+1)*(y-1)^2*(-1+2*x)*(x-1)^3'
    symbol_names = 'mu rho'
    symbol_values = '${mu} ${rho}'
  []
  [forcing_v]
    type = ParsedFunction
    expression = '4*mu/rho*(-1+2*x)*(x^2-6*y*x^2+6*x^2*y^2-x+6*x*y-6*x*y^2+3*y^2-6*y^3+3*y^4)+4*y^3*x^2*(2'
            '*x^2-2*x+1)*(x-1)^2*(-1+2*y)*(y-1)^3'
    symbol_names = 'mu rho'
    symbol_values = '${mu} ${rho}'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_rel_tol = 1e-8
[]

[Outputs]
  [out]
    type = Exodus
    hide = lambda
  []
  csv = true
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2u]
    type = ElementL2Error
    variable = vel_x
    function = exact_u
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2v]
    type = ElementL2Error
    variable = vel_y
    function = exact_v
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2p]
    variable = pressure
    function = exact_p
    type = ElementL2Error
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]
