period=1.25e-3
endtime=${period}
timestep=1.25e-5
surfacetemp=300
sb=5.67e-8
advected_interp_method='upwind'
velocity_interp_method='rc'
rho='rho'
mu='mu'
cp='cp'

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = -.35e-3
  xmax = 0.35e-3
  ymin = -.35e-3
  ymax = .35e-3
  zmin = -.7e-3
  zmax = 0
  nx = 2
  ny = 2
  nz = 2
  displacements = 'disp_x disp_y disp_z'
  uniform_refine = 2
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = vel_x
    v = vel_y
    w = vel_z
    pressure = pressure
    use_displaced_mesh = true
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
  []
  [vel_y]
    type = INSFVVelocityVariable
  []
  [vel_z]
    type = INSFVVelocityVariable
  []
  [T]
    type = INSFVEnergyVariable
  []
  [pressure]
    type = INSFVPressureVariable
  []
  [lambda]
    family = SCALAR
    order = FIRST
  []
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[ICs]
  [T]
    type = FunctionIC
    variable = T
    function = '(${surfacetemp} - 300) / .7e-3 * z + ${surfacetemp}'
  []
[]

[Kernels]
  [disp_x]
    type = Diffusion
    variable = disp_x
  []
  [disp_y]
    type = Diffusion
    variable = disp_y
  []
  [disp_z]
    type = Diffusion
    variable = disp_z
  []
[]

[FVKernels]
  # pressure equation
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    use_displaced_mesh = true
  []
  [pin_zero_pressure]
    type = FVPointValueConstraint
    variable = pressure
    lambda = lambda
    phi0 = 0.0
    point = '0 0 0'
  []

  # momentum equations
  # u equation
  [u_time]
    type = INSFVMomentumTimeDerivative
    variable = vel_x
    rho = ${rho}
    momentum_component = 'x'
    use_displaced_mesh = true
  []
  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'x'
    use_displaced_mesh = true
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = ${mu}
    momentum_component = 'x'
    use_displaced_mesh = true
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
    use_displaced_mesh = true
  []
  # v equation
  [v_time]
    type = INSFVMomentumTimeDerivative
    variable = vel_y
    rho = ${rho}
    momentum_component = 'y'
    use_displaced_mesh = true
  []
  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'y'
    use_displaced_mesh = true
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = ${mu}
    momentum_component = 'y'
    use_displaced_mesh = true
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
    use_displaced_mesh = true
  []
  # w equation
  [w_time]
    type = INSFVMomentumTimeDerivative
    variable = vel_z
    rho = ${rho}
    momentum_component = 'z'
    use_displaced_mesh = true
  []
  [w_advection]
    type = INSFVMomentumAdvection
    variable = vel_z
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'z'
    use_displaced_mesh = true
  []
  [w_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_z
    mu = ${mu}
    momentum_component = 'z'
    use_displaced_mesh = true
  []
  [w_pressure]
    type = INSFVMomentumPressure
    variable = vel_z
    momentum_component = 'z'
    pressure = pressure
    use_displaced_mesh = true
  []

  # energy equation
  [temperature_time]
    type = INSFVEnergyTimeDerivative
    variable = T
    rho = ${rho}
    cp = ${cp}
    use_displaced_mesh = true
  []
  [temperature_advection]
    type = INSFVEnergyAdvection
    variable = T
    use_displaced_mesh = true
  []
  [temperature_conduction]
    type = FVDiffusion
    coeff = 'k'
    variable = T
    use_displaced_mesh = true
  []
[]

[FVBCs]
  # momentum boundary conditions
  [no_slip_x]
    type = INSFVNoSlipWallBC
    variable = vel_x
    boundary = 'bottom right left top back'
    function = 0
  []
  [no_slip_y]
    type = INSFVNoSlipWallBC
    variable = vel_y
    boundary = 'bottom right left top back'
    function = 0
  []
  [no_slip_z]
    type = INSFVNoSlipWallBC
    variable = vel_z
    boundary = 'bottom right left top back'
    function = 0
  []
  [vapor_recoil_x]
    type = INSFVVaporRecoilPressureMomentumFluxBC
    variable = vel_x
    boundary = 'front'
    momentum_component = 'x'
    rc_pressure = rc_pressure
    use_displaced_mesh = true
  []
  [vapor_recoil_y]
    type = INSFVVaporRecoilPressureMomentumFluxBC
    variable = vel_y
    boundary = 'front'
    momentum_component = 'y'
    rc_pressure = rc_pressure
    use_displaced_mesh = true
  []
  [vapor_recoil_z]
    type = INSFVVaporRecoilPressureMomentumFluxBC
    variable = vel_z
    boundary = 'front'
    momentum_component = 'z'
    rc_pressure = rc_pressure
    use_displaced_mesh = true
  []
  # energy boundary conditions
  [T_cold]
    type = FVDirichletBC
    variable = T
    boundary = 'back'
    value = 300
  []
  [radiation_flux]
    type = FVFunctorRadiativeBC
    variable = T
    boundary = 'front'
    emissivity = '1'
    Tinfinity = 300
    stefan_boltzmann_constant = ${sb}
    use_displaced_mesh = true
  []
  [weld_flux]
    type = FVGaussianEnergyFluxBC
    variable = T
    boundary = 'front'
    P0 = 159.96989792079225
    R = 1.8257418583505537e-4
    x_beam_coord = '2e-4 * cos(t * 2 * pi / ${period})'
    y_beam_coord = '2e-4 * sin(t * 2 * pi / ${period})'
    z_beam_coord = 0
    use_displaced_mesh = true
  []
[]

[BCs]
  # displacement boundary conditions
  [x_no_disp]
    type = DirichletBC
    variable = disp_x
    boundary = 'back'
    value = 0
  []
  [y_no_disp]
    type = DirichletBC
    variable = disp_y
    boundary = 'back'
    value = 0
  []
  [z_no_disp]
    type = DirichletBC
    variable = disp_z
    boundary = 'back'
    value = 0
  []
  [displace_x_top]
    type = INSADDisplaceBoundaryBC
    boundary = 'front'
    variable = 'disp_x'
    velocity = 'vel'
    component = 0
    associated_subdomain = 0
  []
  [displace_y_top]
    type = INSADDisplaceBoundaryBC
    boundary = 'front'
    variable = 'disp_y'
    velocity = 'vel'
    component = 1
    associated_subdomain = 0
  []
  [displace_z_top]
    type = INSADDisplaceBoundaryBC
    boundary = 'front'
    variable = 'disp_z'
    velocity = 'vel'
    component = 2
    associated_subdomain = 0
  []
  [displace_x_top_dummy]
    type = INSADDummyDisplaceBoundaryIntegratedBC
    boundary = 'front'
    variable = 'disp_x'
    velocity = 'vel'
    component = 0
  []
  [displace_y_top_dummy]
    type = INSADDummyDisplaceBoundaryIntegratedBC
    boundary = 'front'
    variable = 'disp_y'
    velocity = 'vel'
    component = 1
  []
  [displace_z_top_dummy]
    type = INSADDummyDisplaceBoundaryIntegratedBC
    boundary = 'front'
    variable = 'disp_z'
    velocity = 'vel'
    component = 2
  []
[]

[FunctorMaterials]
  [steel]
    type = AriaLaserWeld304LStainlessSteelFunctorMaterial
    temperature = T
    beta = 1e7
  []
  # [const]
  #   type = GenericConstantMaterial
  #   prop_names = 'abs sb_constant'
  #   prop_values = '1 ${sb}'
  #   use_displaced_mesh = true
  # []
  [disp_vec_value_and_dot]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'disp_vec'
    prop_values = 'disp_x disp_y disp_z'
  []
  [vel]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'vel'
    prop_values = 'vel_x vel_y vel_z'
  []
  [h]
    type = INSFVEnthalpyMaterial
    rho = 'rho'
    temperature = T
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_type -mat_mffd_err'
    petsc_options_value = 'lu       NONZERO               strumpack                  1e-6'
  []
[]

[Executioner]
  type = Transient
  end_time = ${endtime}
  dtmin = 1e-8
  dtmax = ${timestep}
  petsc_options = '-snes_converged_reason -ksp_converged_reason -options_left'
  solve_type = 'PJFNK'
  line_search = 'none'
  nl_max_its = 12
  l_max_its = 100
  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 7
    dt = ${timestep}
    linear_iteration_ratio = 1e6
    growth_factor = 1.5
  []
[]

[Outputs]
  [exodus]
    type = Exodus
    output_material_properties = true
    show_material_properties = 'mu'
  []
  checkpoint = true
  perf_graph = true
[]

[Debug]
  show_var_residual_norms = true
[]


[Adaptivity]
  marker = combo
  max_h_level = 4

  [Indicators]
    [error_T]
      type = GradientJumpIndicator
      variable = T
    []
    [error_dispz]
      type = GradientJumpIndicator
      variable = disp_z
    []
  []

  [Markers]
    [errorfrac_T]
      type = ErrorFractionMarker
      refine = 0.4
      coarsen = 0.2
      indicator = error_T
    []
    [errorfrac_dispz]
      type = ErrorFractionMarker
      refine = 0.4
      coarsen = 0.2
      indicator = error_dispz
    []
    [combo]
      type = ComboMarker
      markers = 'errorfrac_T errorfrac_dispz'
    []
  []
[]

[Postprocessors]
  [num_dofs]
    type = NumDOFs
    system = 'NL'
  []
  [nl]
    type = NumNonlinearIterations
  []
  [tot_nl]
    type = CumulativeValuePostprocessor
    postprocessor = 'nl'
  []
[]
