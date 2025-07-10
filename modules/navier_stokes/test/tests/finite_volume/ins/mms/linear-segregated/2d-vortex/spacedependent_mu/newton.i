rho = 1.0

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
[]

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

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 1
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 1
  []
  [pressure]
    type = INSFVPressureVariable
  []
  [lambda]
    family = SCALAR
    order = FIRST
  []
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = 'average'
    velocity_interp_method = 'rc'
    rho = ${rho}
  []
  [mean_zero_pressure]
    type = FVIntegralValueConstraint
    variable = pressure
    lambda = lambda
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    advected_interp_method = 'average'
    velocity_interp_method = 'rc'
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = 'mu'
    momentum_component = 'x'
    complete_expansion = false
    u = vel_x
    v = vel_y
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
  []
  [u_forcing]
    type = INSFVBodyForce
    variable = vel_x
    functor = forcing_u
    momentum_component = 'x'
  []

  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    advected_interp_method = 'average'
    velocity_interp_method = 'rc'
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = 'mu'
    momentum_component = 'y'
    complete_expansion = false
    u = vel_x
    v = vel_y
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
  []
  [v_forcing]
    type = INSFVBodyForce
    variable = vel_y
    functor = forcing_v
    momentum_component = 'y'
  []
[]

[FVBCs]
  [no-slip-wall-u]
    type = INSFVNoSlipWallBC
    boundary = 'left right top bottom'
    variable = vel_x
    function = '0'
  []
  [no-slip-wall-v]
    type = INSFVNoSlipWallBC
    boundary = 'left right top bottom'
    variable = vel_y
    function = '0'
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
  [mu]
    type = ParsedFunction
    expression = '1+(x-1)*x*(y-1)*y'
  []
  [forcing_u]
    type = ParsedFunction
    expression = '-(2*x-1)*y*(y-1)*(2*x-6*x^2+4*x^3)*(2*y-6*y^2+4*y^3)'
                 '-(1+x*(x-1)*y*(y-1))*(2*y-6*y^2+4*y^3)*(2-12*x+12*x^2)'
                 '-(2*y-1)*x*(x-1)*(x^2*(1-x)^2*(2-12*y+12*y^2))'
                 '-(1+x*(x-1)*y*(y-1))*(x^2*(1-x)^2*(-12+24*y))'
                 '+1-2*x+rho*4*x^3*y^2*(2*y^2-2*y+1)*(y-1)^2*(-1+2*x)*(x-1)^3'
    symbol_names = 'rho'
    symbol_values = '${rho}'
  []
  [forcing_v]
    type = ParsedFunction
    expression = '(2*y-1)*x*(x-1)*(2*y-6*y^2+4*y^3)*(2*x-6*x^2+4*x^3)'
                 '+(1+x*(x-1)*y*(y-1))*(2-12*y+12*y^2)*(2*x-6*x^2+4*x^3)'
                 '+(2*x-1)*y*(y-1)*(y^2*(1-y)^2*(2-12*x+12*x^2))'
                 '+(1+x*(x-1)*y*(y-1))*(y^2*(1-y)^2*(-12+24*x))'
                 '+rho*4*y^3*x^2*(2*x^2-2*x+1)*(x-1)^2*(-1+2*y)*(y-1)^3'
    symbol_names = 'rho'
    symbol_values = '${rho}'
  []
  [forcing_u_deviatoric]
    type = ParsedFunction
    expression = '-2*(2*x-1)*y*(y-1)*(2*x-6*x^2+4*x^3)*(2*y-6*y^2+4*y^3)'
                 '-2*(1+x*(x-1)*y*(y-1))*(2*y-6*y^2+4*y^3)*(2-12*x+12*x^2)'
                 '-(2*y-1)*x*(x-1)*(x^2*(1-x)^2*(2-12*y+12*y^2)-y^2*(1-y)^2*(2-12*x+12*x^2))'
                 '-(1+x*(x-1)*y*(y-1))*(x^2*(1-x)^2*(-12+24*y)-(2*y-6*y^2+4*y^3)*(2-12*x+12*x^2))'
                 '+1-2*x+rho*4*x^3*y^2*(2*y^2-2*y+1)*(y-1)^2*(-1+2*x)*(x-1)^3'
    symbol_names = 'rho'
    symbol_values = '${rho}'
  []
  [forcing_v_deviatoric]
    type = ParsedFunction
    expression = '2*(2*y-1)*x*(x-1)*(2*y-6*y^2+4*y^3)*(2*x-6*x^2+4*x^3)'
                 '+2*(1+x*(x-1)*y*(y-1))*(2-12*y+12*y^2)*(2*x-6*x^2+4*x^3)'
                 '-(2*x-1)*y*(y-1)*(x^2*(1-x)^2*(2-12*y+12*y^2)-y^2*(1-y)^2*(2-12*x+12*x^2))'
                 '-(1+x*(x-1)*y*(y-1))*(-y^2*(1-y)^2*(-12+24*x)+(2*x-6*x^2+4*x^3)*(2-12*y+12*y^2))'
                 '+rho*4*y^3*x^2*(2*x^2-2*x+1)*(x-1)^2*(-1+2*y)*(y-1)^3'
    symbol_names = 'rho'
    symbol_values = '${rho}'
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
    type = ElementL2FunctorError
    approximate = vel_x
    exact = exact_u
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2v]
    type = ElementL2FunctorError
    approximate = vel_y
    exact = exact_v
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2p]
    approximate = pressure
    exact = exact_p
    type = ElementL2FunctorError
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]
