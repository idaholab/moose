mu=1
rho=1

# This input file tests whether we can converge to the semi-analytical
# solution for flow in a 2D wedge.
[GlobalParams]
  velocity_interp_method = 'rc'
  advected_interp_method = 'average'
  rhie_chow_user_object = 'rc'
  alpha_degrees = 15
  Re = 30
  K = -9.78221333616
  f = f_theta
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    # file = wedge_4x6.e
    file = wedge_8x12.e
    # file = wedge_16x24.e
    # file = wedge_32x48.e
    # file = wedge_64x96.e
  []
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
  []
  [vel_y]
    type = INSFVVelocityVariable
  []
  [pressure]
    type = INSFVPressureVariable
  []
  [lambda]
    family = SCALAR
    order = FIRST
  []
[]

[AuxVariables]
  [U]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
[]

[AuxKernels]
  [mag]
    type = VectorMagnitudeAux
    variable = U
    x = vel_x
    y = vel_y
  []
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    rho = ${rho}
  []

  [mean_zero_pressure]
    type = FVIntegralValueConstraint
    variable = pressure
    lambda = lambda
    phi0 = 0.0
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    rho = ${rho}
    momentum_component = 'x'
  []

  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = 'mu'
    momentum_component = 'x'
  []

  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
  []

  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    rho = ${rho}
    momentum_component = 'y'
  []

  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = mu
    momentum_component = y
  []

  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = y
    pressure = pressure
  []
[]

[FVBCs]

  [no_slip_x]
    type = INSFVNoSlipWallBC
    variable = vel_x
    boundary = 'top_wall bottom_wall'
    function = 0
  []

  [no_slip_y]
    type = INSFVNoSlipWallBC
    variable = vel_y
    boundary = 'top_wall bottom_wall'
    function = 0
  []

  [inlet_x]
    type = INSFVInletVelocityBC
    variable = vel_x
    boundary = 'inlet outlet'
    function = vel_x_exact
  []

  [inlet_y]
    type = INSFVInletVelocityBC
    variable = vel_y
    boundary = 'inlet outlet'
    function = vel_y_exact
  []
[]

[Functions]
[./f_theta]
  # Non-dimensional solution values f(eta), 0 <= eta <= 1 for
  # alpha=15 deg, Re=30.  Note: this introduces an input file
  # ordering dependency: this Function must appear *before* the two
  # functions below which use it since apparently proper dependency
  # resolution is not done in this scenario.
  type = PiecewiseLinear
  data_file = 'f.csv'
  format = 'columns'
[../]
  [vel_x_exact]
    type = WedgeFunction
    var_num = 0
    mu = 1
    rho = 1
  []
  [vel_y_exact]
    type = WedgeFunction
    var_num = 1
    mu = 1
    rho = 1
  []
[]

[Materials]
  [mu]
    type = ADGenericFunctorMaterial
    prop_names = 'mu'
    prop_values = '${mu}'
  []
[]

[Preconditioning]
  [./SMP_PJFNK]
    type = SMP
    full = true
    solve_type = NEWTON
  [../]
[]

[Executioner]
  type = Transient
  dt = 1.e-2
  dtmin = 1.e-2
  num_steps = 5
  petsc_options_iname = '-ksp_gmres_restart -pc_type -sub_pc_type -sub_pc_factor_levels'
  petsc_options_value = '300                bjacobi  ilu          4'
  line_search = none
  nl_rel_tol = 1e-13
  nl_abs_tol = 1e-11
  nl_max_its = 10
  l_tol = 1e-6
  l_max_its = 300
[]

[Outputs]
  exodus = true
[]
