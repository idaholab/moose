###############################################################################
# Validation test based on Hibiki and Ishii experiment [1] reported in Figure 3
# [1] Hibiki, T., & Ishii, M. (2000). One-group interfacial area transport of bubbly flows in vertical round tubes.
# International Journal of Heat and Mass Transfer, 43(15), 2711-2726.
###############################################################################

mu = 1.0
rho = 1000.0
mu_d = 1.0
rho_d = 1.0
l = ${fparse 50.8/1000.0}
U = 0.491230114
dp = 0.001
inlet_phase_2 = 0.049
advected_interp_method = 'upwind'
velocity_interp_method = 'rc'
mass_exchange_coeff = 0.0
inlet_interface_area = ${fparse 6.0*inlet_phase_2/dp}

outlet_pressure = 1e6

[GlobalParams]
  rhie_chow_user_object = 'rc'
  density_interp_method = 'average'
  mu_interp_method = 'average'
[]

[Problem]
  identify_variable_groups_in_nl = false
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
  coord_type = 'RZ'
  rz_coord_axis = 'X'
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = '${fparse l * 60}'
    ymin = 0
    ymax = '${fparse l / 2}'
    nx = 20
    ny = 5
  []
  uniform_refine = 0
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 0
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 0
  []
  [pressure]
    type = INSFVPressureVariable
  []
  [phase_2]
    type = INSFVScalarFieldVariable
    initial_condition = ${inlet_phase_2}
  []
  [interface_area]
    type = INSFVScalarFieldVariable
    initial_condition = ${inlet_interface_area}
  []
[]

[FVKernels]

  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
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
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = 'rho_mixture'
    momentum_component = 'x'
  []
  [u_drift]
    type = WCNSFV2PMomentumDriftFlux
    variable = vel_x
    rho_d = ${rho_d}
    fd = 'rho_mixture_var'
    u_slip = 'vel_slip_x'
    v_slip = 'vel_slip_y'
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = 'mu_mixture'
    limit_interpolation = true
    momentum_component = 'x'
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
  []

  [v_time]
    type = INSFVMomentumTimeDerivative
    variable = vel_y
    rho = 'rho_mixture'
    momentum_component = 'y'
  []
  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = 'rho_mixture'
    momentum_component = 'y'
  []
  [v_drift]
    type = WCNSFV2PMomentumDriftFlux
    variable = vel_y
    rho_d = ${rho_d}
    fd = 'rho_mixture_var'
    u_slip = 'vel_slip_x'
    v_slip = 'vel_slip_y'
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = 'mu_mixture'
    limit_interpolation = true
    momentum_component = 'y'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
  []

  [phase_2_time]
    type = FVFunctorTimeKernel
    variable = phase_2
    functor = phase_2
  []
  [phase_2_advection]
    type = INSFVScalarFieldAdvection
    variable = phase_2
    u_slip = 'vel_x'
    v_slip = 'vel_y'
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = 'upwind'
  []
  [phase_2_diffusion]
    type = FVDiffusion
    variable = phase_2
    coeff = 1.0
  []
  [phase_2_src]
    type = NSFVMixturePhaseInterface
    variable = phase_2
    phase_coupled = phase_1
    alpha = ${mass_exchange_coeff}
  []

  [interface_area_time]
    type = FVFunctorTimeKernel
    variable = interface_area
    functor = interface_area
  []
  [interface_area_advection]
    type = INSFVScalarFieldAdvection
    variable = interface_area
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = 'upwind'
  []
  [interface_area_diffusion]
    type = FVDiffusion
    variable = interface_area
    coeff = 0.1
  []
  [interface_area_source_sink]
    type = WCNSFV2PInterfaceAreaSourceSink
    variable = interface_area
    u = 'vel_x'
    v = 'vel_y'
    L = ${fparse l/2}
    rho = 'rho_mixture'
    rho_d = 'rho'
    pressure = 'pressure'
    k_c = '${fparse mass_exchange_coeff}'
    fd = 'phase_2'
    sigma = 1e-3
    cutoff_fraction = 0.0
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_x
    functor = '${U}'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_y
    functor = '0'
  []
  [walls-u]
    type = INSFVNoSlipWallBC
    boundary = 'top'
    variable = vel_x
    function = 0
  []
  [walls-v]
    type = INSFVNoSlipWallBC
    boundary = 'top'
    variable = vel_y
    function = 0
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = '${outlet_pressure}'
  []
  [inlet_phase_2]
    type = FVDirichletBC
    boundary = 'left'
    variable = phase_2
    value = ${inlet_phase_2}
  []
  [inlet_interface_area]
    type = FVDirichletBC
    boundary = 'left'
    variable = interface_area
    value = ${inlet_interface_area}
  []

  [symmetry-u]
    type = PINSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = vel_x
    u = vel_x
    v = vel_y
    mu = 'mu_mixture'
    momentum_component = 'x'
  []
  [symmetry-v]
    type = PINSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = vel_y
    u = vel_x
    v = vel_y
    mu = 'mu_mixture'
    momentum_component = 'y'
  []
  [symmetry-p]
    type = INSFVSymmetryPressureBC
    boundary = 'bottom'
    variable = pressure
  []
  [symmetry-phase-2]
    type = INSFVSymmetryScalarBC
    boundary = 'bottom'
    variable = phase_2
  []
  [symmetry-interface-area]
    type = INSFVSymmetryScalarBC
    boundary = 'bottom'
    variable = interface_area
  []
[]

[AuxVariables]
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

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
  []
[]

[FunctorMaterials]
  [bubble_properties]
    type = GeneralFunctorFluidProps
    fp = 'fp'
    pressure = 'pressure'
    T_fluid = 300.0
    speed = 1.0
    characteristic_length = 1.0
    porosity = 1.0
    output_properties = 'rho'
    outputs = 'out'
  []
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
    phase_2_names = '${rho} ${mu}'
    phase_1_names = 'rho ${mu_d}'
    prop_names = 'rho_mixture mu_mixture'
    phase_1_fraction = 'phase_2'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  nl_abs_tol = 1e-7
  dt = 0.1
  end_time = 1.0
  nl_max_its = 10
  line_search = 'none'
[]

[Debug]
  show_var_residual_norms = true
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_shift_type'
    petsc_options_value = 'lu       NONZERO'
  []
[]

[Outputs]
  [out]
    type = Exodus
  []
[]

[Postprocessors]
  [Re]
    type = ParsedPostprocessor
    expression = '${rho} * ${l} * ${U}'
    pp_names = ''
  []
  [rho_outlet]
    type = SideAverageValue
    boundary = 'right'
    variable = 'rho_mixture_var'
  []
[]
