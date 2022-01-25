mu=1.0
rho=1.0

[Problem]
  coord_type = 'XYZ'
  error_on_jacobian_nonzero_reallocation = true
[]

[Mesh]
  [gen_mesh]
    type = FileMeshGenerator
    file = skewed.msh
  []
[]

[Modules]
  [NavierStokesFV]
    simulation_type = 'steady-state'
    compressibility = 'incompressible'
    porous_medium_treatment = false
    add_energy_equation = false

    density = 'rho'
    dynamic_viscosity = 'mu'
    gravity = '0 0 0'

    wall_boundaries = 'left top right bottom'
    momentum_wall_types = 'noslip noslip noslip noslip'
    pin_pressure = true

    pinned_pressure_type = point-value
    pinned_pressure_point = '0.001 0.6 0.0'
    pinned_pressure_value = 100
  []
[]

[FVKernels]
  [u_forcing]
    type = INSFVBodyForce
    variable = vel_x
    functor = forcing_u
    momentum_component = 'x'
    rhie_chow_user_object = ins_rhie_chow_interpolator
  []
  [v_forcing]
    type = INSFVBodyForce
    variable = vel_y
    functor = forcing_v
    momentum_component = 'y'
    rhie_chow_user_object = ins_rhie_chow_interpolator
  []
[]

[Functions]
  [exact_u]
    type = ParsedFunction
    value = 'x^2*(1-x)^2*(2*y-6*y^2+4*y^3)'
  []
  [exact_v]
    type = ParsedFunction
    value = '-y^2*(1-y)^2*(2*x-6*x^2+4*x^3)'
  []
  [exact_p]
    type = ParsedFunction
    value = 'x*(1-x)-2/12'
  []
  [forcing_u]
    type = ADParsedFunction
    value = '-4*mu/rho*(-1+2*y)*(y^2-6*x*y^2+6*x^2*y^2-y+6*x*y-6*x^2*y+3*x^2-6*x^3+3*x^4)+1-2*x+4*x^3*y^2*(2*y^2-2*y+1)*(y-1)^2*(-1+2*x)*(x-1)^3'
    vars = 'mu rho'
    vals = '${mu} ${rho}'
  []
  [forcing_v]
    type = ADParsedFunction
    value = '4*mu/rho*(-1+2*x)*(x^2-6*y*x^2+6*x^2*y^2-x+6*x*y-6*x*y^2+3*y^2-6*y^3+3*y^4)+4*y^3*x^2*(2*x^2-2*x+1)*(x-1)^2*(-1+2*y)*(y-1)^3'
    vars = 'mu rho'
    vals = '${mu} ${rho}'
  []
[]

[Materials]
  [const]
    type = ADGenericFunctorMaterial
    prop_names = 'rho mu'
    prop_values = '${rho} ${mu}'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'bjacobi      30                 lu           NONZERO'
  line_search = 'none'
  nl_rel_tol = 1e-8
[]

[Debug]
show_actions = true
[]

[Outputs]
  exodus = true
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
