mu = 1.1
rho = 1.2
advected_interp_method = 'average'
velocity_interp_method = 'rc'

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = vel_x
    v = vel_y
    pressure = pressure
  []
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 0.0
    two_term_boundary_expansion = false
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 0.0
    two_term_boundary_expansion = false
  []
  [pressure]
    type = INSFVPressureVariable
    initial_condition = 0.0
    two_term_boundary_expansion = false
  []
  [lambda]
    family = SCALAR
    order = FIRST
  []
[]


[FVKernels]
  [mean_zero_pressure]
    type = FVPointValueConstraint
    variable = pressure
    lambda = lambda
    point = '0.5 0.5 0'
    phi0 = 0.25
  []
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
  []
  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = ${mu}
    momentum_component = 'x'
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
  []
  [u_forcing]
    type = FVBodyForce
    variable = vel_x
    function = forcing_u
  []
  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = ${mu}
    momentum_component = 'y'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
  []
  [v_forcing]
    type = FVBodyForce
    variable = vel_y
    function = forcing_v
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left right top'
    variable = vel_x
    function = 'exact_u'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left right top'
    variable = vel_y
    function = 'exact_v'
  []
  [axis-u]
    type = INSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = vel_x
    u = vel_x
    v = vel_y
    mu = ${mu}
    momentum_component = x
  []
  [axis-v]
    type = INSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = vel_y
    u = vel_x
    v = vel_y
    mu = ${mu}
    momentum_component = y
  []
  [axis-p]
    type = INSFVSymmetryPressureBC
    boundary = 'bottom'
    variable = pressure
  []
[]

[Functions]
  [exact_u]
    type = ParsedFunction
    expression = '(x-x^2)*(1-2*y^2)'
  []
  [exact_v]
    type = ParsedFunction
    expression = '-(y-2/3*y^3)*(1-2*x)'
  []
  [exact_p]
    type = ParsedFunction
    expression = 'y^2'
  []
  [forcing_u]
    type = ParsedFunction
    # expression = '2*mu*(1-2*y^2)+rho*(x-x^2)*(1-2*y^2)*(1-2*x)*(1-2*y^2)+rho*(-y+2/3*y^3)*(1-2*x)*(x-x^2)*(-4*y)'
    expression = '-mu*(4*x^2-4*x+4*y^2-2)+rho/3*x*(x-1)*(2*x-1)*(4*y^4+3)'
    symbol_names = 'mu rho'
    symbol_values = '${mu} ${rho}'
  []
  [forcing_v]
    type = ParsedFunction
    # expression = '-mu*4*y*(1-2*x)+rho*(x-x^2)*(1-2*y^2)*(-y+2/3*y^3)*(-2)+rho*(-y+2/3*y^3)*(-1+2*y^2)*(1-2*x)+2*y'
    expression = '-mu*4*y*(1-2*x)+rho/3*y*(2*y^2-3)*(2*y^2-1)*(2*x^2-2*x+1)+2*y'
    symbol_names = 'mu rho'
    symbol_values = '${mu} ${rho}'
  []
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-8
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
[]

[Outputs]
  exodus = true
  csv = false
  perf_graph = false
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    # outputs = 'csv'
    execute_on = TIMESTEP_END
  []
  [L2u]
    type = ElementL2FunctorError
    approximate = vel_x
    exact = exact_u
    # outputs = 'csv'
    execute_on = TIMESTEP_END
  []
  [L2v]
    type = ElementL2FunctorError
    approximate = vel_y
    exact = exact_v
    # outputs = 'csv'
    execute_on = TIMESTEP_END
  []
  [L2p]
    approximate = pressure
    exact = exact_p
    type = ElementL2FunctorError
    # outputs = 'csv'
    execute_on = TIMESTEP_END
  []
[]

