period=1.25e-3
endtime=${period}
timestep=1.25e-5
surfacetemp=300
sb=5.67e-8

[GlobalParams]
  temperature = T
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

[Variables]
  [vel]
    family = LAGRANGE_VEC
  []
  [T]
  []
  [p]
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
  [mass]
    type = INSADMass
    variable = p
    use_displaced_mesh = true
  []
  [mass_pspg]
    type = INSADMassPSPG
    variable = p
    use_displaced_mesh = true
  []
  [momentum_time]
    type = INSADMomentumTimeDerivative
    variable = vel
    use_displaced_mesh = true
  []
  [momentum_advection]
    type = INSADMomentumAdvection
    variable = vel
    use_displaced_mesh = true
  []
  [momentum_mesh_advection]
    type = INSADMomentumMeshAdvection
    variable = vel
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    use_displaced_mesh = true
  []
  [momentum_viscous]
    type = INSADMomentumViscous
    variable = vel
    use_displaced_mesh = true
  []
  [momentum_pressure]
    type = INSADMomentumPressure
    variable = vel
    pressure = p
    integrate_p_by_parts = true
    use_displaced_mesh = true
  []
  [momentum_supg]
    type = INSADMomentumSUPG
    variable = vel
    material_velocity = relative_velocity
    use_displaced_mesh = true
  []
  [temperature_time]
    type = INSADHeatConductionTimeDerivative
    variable = T
    use_displaced_mesh = true
  []
  [temperature_advection]
    type = INSADEnergyAdvection
    variable = T
    use_displaced_mesh = true
  []
  [temperature_mesh_advection]
    type = INSADEnergyMeshAdvection
    variable = T
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    use_displaced_mesh = true
  []
  [temperature_conduction]
    type = ADHeatConduction
    variable = T
    thermal_conductivity = 'k'
    use_displaced_mesh = true
  []
  [temperature_supg]
    type = INSADEnergySUPG
    variable = T
    velocity = vel
    use_displaced_mesh = true
  []
[]

[BCs]
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
  [no_slip]
    type = ADVectorFunctionDirichletBC
    variable = vel
    boundary = 'bottom right left top back'
  []
  [T_cold]
    type = DirichletBC
    variable = T
    boundary = 'back'
    value = 300
  []
  [radiation_flux]
    type = FunctionRadiativeBC
    variable = T
    boundary = 'front'
    emissivity_function = '1'
    Tinfinity = 300
    stefan_boltzmann_constant = ${sb}
    use_displaced_mesh = true
  []
  [weld_flux]
    type = GaussianEnergyFluxBC
    variable = T
    boundary = 'front'
    P0 = 159.96989792079225
    R = 1.8257418583505537e-4
    x_beam_coord = '2e-4 * cos(t * 2 * pi / ${period})'
    y_beam_coord = '2e-4 * sin(t * 2 * pi / ${period})'
    z_beam_coord = 0
    use_displaced_mesh = true
  []
  [vapor_recoil]
    type = INSADVaporRecoilPressureMomentumFluxBC
    variable = vel
    boundary = 'front'
    use_displaced_mesh = true
  []
  [surface_tension]
    type = INSADSurfaceTensionBC
    variable = vel
    boundary = 'front'
    use_displaced_mesh = true
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

[Materials]
  [ins_mat]
    type = INSADStabilized3Eqn
    velocity = vel
    pressure = p
    temperature = T
    use_displaced_mesh = true
  []
  [steel]
    type = AriaLaserWeld304LStainlessSteel
    temperature = T
    beta = 1e7
    use_displaced_mesh = true
  []
  [steel_boundary]
    type = AriaLaserWeld304LStainlessSteelBoundary
    boundary = 'front'
    temperature = T
    use_displaced_mesh = true
  []
  [const]
    type = GenericConstantMaterial
    prop_names = 'abs sb_constant'
    prop_values = '1 ${sb}'
    use_displaced_mesh = true
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_type'
    petsc_options_value = 'lu       NONZERO               strumpack'
  []
[]

[Executioner]
  type = Transient
  end_time = ${endtime}
  dtmin = 1e-8
  dtmax = ${timestep}
  petsc_options = '-snes_converged_reason -ksp_converged_reason -options_left'
  solve_type = 'NEWTON'
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
