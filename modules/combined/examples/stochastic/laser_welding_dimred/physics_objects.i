[ICs]
  [T]
    type = FunctionIC
    variable = T
    function = '(${surfacetemp} - 300) / ${thickness} * y + ${surfacetemp}'
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
    boundary = 'bottom'
    value = 0
  []
  [y_no_disp]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom'
    value = 0
  []
  [no_slip]
    type = ADVectorFunctionDirichletBC
    variable = vel
    boundary = 'bottom right left'
  []
  [T_cold]
    type = DirichletBC
    variable = T
    boundary = 'bottom'
    value = 300
  []
  [radiation_flux]
    type = FunctionRadiativeBC
    variable = T
    boundary = 'top'
    emissivity_function = '1'
    Tinfinity = 300
    stefan_boltzmann_constant = 5.67e-8
    use_displaced_mesh = true
  []
  [weld_flux]
    type = GaussianEnergyFluxBC
    variable = T
    boundary = 'top'
    P0 = ${power}
    R = ${R}
    x_beam_coord = '${scanning_speed}*t'
    y_beam_coord = '0'
    use_displaced_mesh = true
  []
  [vapor_recoil]
    type = INSADVaporRecoilPressureMomentumFluxBC
    variable = vel
    boundary = 'top'
    use_displaced_mesh = true
  []
  [surface_tension]
    type = INSADSurfaceTensionBC
    variable = vel
    boundary = 'top'
    use_displaced_mesh = true
  []
  [displace_x_top]
    type = INSADDisplaceBoundaryBC
    boundary = 'top'
    variable = 'disp_x'
    velocity = 'vel'
    component = 0
    associated_subdomain = 0
  []
  [displace_y_top]
    type = INSADDisplaceBoundaryBC
    boundary = 'top'
    variable = 'disp_y'
    velocity = 'vel'
    component = 1
    associated_subdomain = 0
  []
  [displace_x_top_dummy]
    type = INSADDummyDisplaceBoundaryIntegratedBC
    boundary = 'top'
    variable = 'disp_x'
    velocity = 'vel'
    component = 0
  []
  [displace_y_top_dummy]
    type = INSADDummyDisplaceBoundaryIntegratedBC
    boundary = 'top'
    variable = 'disp_y'
    velocity = 'vel'
    component = 1
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
    type = LaserWeld316LStainlessSteel
    temperature = T
    use_displaced_mesh = true
  []
  [steel_boundary]
    type = LaserWeld316LStainlessSteelBoundary
    boundary = 'top'
    temperature = T
    use_displaced_mesh = true
  []
  [const]
    type = GenericConstantMaterial
    prop_names = 'abs sb_constant'
    prop_values = '1 5.67e-8'
    use_displaced_mesh = true
  []
[]
