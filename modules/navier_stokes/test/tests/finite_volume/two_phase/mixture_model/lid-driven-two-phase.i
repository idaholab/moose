mu = 1.0
rho = 1.0e3
mu_d = 0.3
rho_d = 1.0
dp = 0.01
U_lid = 0.1
g = -9.81

[GlobalParams]
  velocity_interp_method = 'rc'
  advected_interp_method = 'upwind'
  rhie_chow_user_object = 'rc'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = .1
    ymin = 0
    ymax = .1
    nx = 10
    ny = 10
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
  [phase_2]
    type = INSFVScalarFieldVariable
  []
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = vel_x
    v = vel_y
    pressure = pressure
  []
  [pin_pressure]
    type = NSPressurePin
    variable = pressure
    pin_type = point-value
    point = '0 0 0'
  []
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    rho = 'rho_mixture'
  []

  [u_time]
    type = INSFVMomentumTimeDerivative
    variable = vel_x
    rho = 'rho_mixture'
    momentum_component = 'x'
  []
  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    rho = 'rho_mixture'
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = 'mu_mixture'
    momentum_component = 'x'
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
  []
  [u_buoyant]
    type = INSFVMomentumGravity
    variable = vel_x
    rho = 'rho_mixture'
    momentum_component = 'x'
    gravity = '0 ${g} 0'
  []
  # NOTE: the friction terms for u and v are missing

  [v_time]
    type = INSFVMomentumTimeDerivative
    variable = vel_y
    rho = 'rho_mixture'
    momentum_component = 'y'
  []
  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    rho = 'rho_mixture'
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = 'mu_mixture'
    momentum_component = 'y'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
  []
  [v_buoyant]
    type = INSFVMomentumGravity
    variable = vel_y
    rho = 'rho_mixture'
    momentum_component = 'y'
    gravity = '0 ${g} 0'
  []

  [phase_2_time]
    type = FVFunctorTimeKernel
    variable = phase_2
  []
  [phase_2_advection]
    type = INSFVScalarFieldAdvection
    variable = phase_2
    u_slip = 'vel_slip_x'
    v_slip = 'vel_slip_y'
  []
  [phase_2_diffusion]
    type = FVDiffusion
    variable = phase_2
    coeff = 1e-3
  []
[]

[FVBCs]
  [top_x]
    type = INSFVNoSlipWallBC
    variable = vel_x
    boundary = 'top'
    function = ${U_lid}
  []

  [no_slip_x]
    type = INSFVNoSlipWallBC
    variable = vel_x
    boundary = 'left right bottom'
    function = 0
  []

  [no_slip_y]
    type = INSFVNoSlipWallBC
    variable = vel_y
    boundary = 'left right top bottom'
    function = 0
  []

  [bottom_phase_2]
    type = FVDirichletBC
    variable = phase_2
    boundary = 'bottom'
    value = 1.0
  []

  [top_phase_2]
    type = FVDirichletBC
    variable = phase_2
    boundary = 'top'
    value = 0.0
  []
[]

[AuxVariables]
  [U]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
  [drag_coefficient]
    type = MooseVariableFVReal
  []
  [rho_mixture_var]
    type = MooseVariableFVReal
  []
  [mu_mixture_var]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [mag]
    type = VectorMagnitudeAux
    variable = U
    x = vel_x
    y = vel_y
  []
  [populate_cd]
    type = FunctorAux
    variable = drag_coefficient
    functor = 'Darcy_coefficient'
  []
  [populate_rho_mixture_var]
    type = FunctorAux
    variable = rho_mixture_var
    functor = 'rho_mixture'
  []
  [populate_mu_mixture_var]
    type = FunctorAux
    variable = mu_mixture_var
    functor = 'mu_mixture'
  []
[]

[FunctorMaterials]
  [populate_u_slip]
    type = WCNSFV2PSlipVelocityFunctorMaterial
    slip_velocity_name = 'vel_slip_x'
    momentum_component = 'x'
    u = 'vel_x'
    v = 'vel_y'
    rho = ${rho}
    mu = 'mu_mixture'
    rho_d = ${rho_d}
    particle_diameter = ${dp}
    linear_coef_name = 'Darcy_coefficient'
    gravity = '0 ${g} 0'
  []
  [populate_v_slip]
    type = WCNSFV2PSlipVelocityFunctorMaterial
    slip_velocity_name = 'vel_slip_y'
    momentum_component = 'y'
    u = 'vel_x'
    v = 'vel_y'
    rho = ${rho}
    mu = 'mu_mixture'
    rho_d = ${rho_d}
    particle_diameter = ${dp}
    linear_coef_name = 'Darcy_coefficient'
    gravity = '0 ${g} 0'
  []
  [compute_phase_1]
    type = ADParsedFunctorMaterial
    property_name = phase_1
    functor_names = 'phase_2'
    expression = '1 - phase_2'
  []
  [CD]
    type = NSFVDispersePhaseDragFunctorMaterial
    rho = 'rho_mixture'
    mu = mu_mixture
    u = 'vel_x'
    v = 'vel_y'
    particle_diameter = ${dp}
  []
  [mixing_material]
    type = NSFVMixtureFunctorMaterial
    phase_1_names = '${rho_d} ${mu_d}'
    phase_2_names = '${rho} ${mu}'
    prop_names = 'rho_mixture mu_mixture'
    phase_1_fraction = 'phase_2'
  []
[]

[Postprocessors]
  [average_void]
    type = ElementAverageValue
    variable = 'phase_2'
  []
  [max_y_velocity]
    type = ElementExtremeValue
    variable = 'vel_y'
    value_type = max
  []
  [min_y_velocity]
    type = ElementExtremeValue
    variable = 'vel_y'
    value_type = min
  []
  [max_x_velocity]
    type = ElementExtremeValue
    variable = 'vel_x'
    value_type = max
  []
  [min_x_velocity]
    type = ElementExtremeValue
    variable = 'vel_x'
    value_type = min
  []
  [max_x_slip_velocity]
    type = ElementExtremeFunctorValue
    functor = 'vel_slip_x'
    value_type = max
  []
  [max_y_slip_velocity]
    type = ElementExtremeFunctorValue
    functor = 'vel_slip_y'
    value_type = max
  []
  [max_drag_coefficient]
    type = ElementExtremeFunctorValue
    functor = 'drag_coefficient'
    value_type = max
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 7
    iteration_window = 2
    growth_factor = 2.0
    cutback_factor = 0.5
    dt = 1e-3
  []
  nl_max_its = 10
  nl_rel_tol = 1e-03
  nl_abs_tol = 1e-9
  l_max_its = 5
  end_time = 1e8
[]

[Outputs]
  exodus = false
  [CSV]
    type = CSV
    execute_on = 'FINAL'
  []
[]
