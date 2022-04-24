[Mesh]
  second_order = true
  inactive = 'mesh internal_boundary_bot internal_boundary_top'
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1'
    dy = '1 1 1'
    ix = '5'
    iy = '5 5 5'
    subdomain_id = '1
                    2
                    3'
  []
  [internal_boundary_bot]
    type = SideSetsBetweenSubdomainsGenerator
    input = mesh
    new_boundary = 'internal_bot'
    primary_block = 1
    paired_block = 2
  []
  [internal_boundary_top]
    type = SideSetsBetweenSubdomainsGenerator
    input = internal_boundary_bot
    new_boundary = 'internal_top'
    primary_block = 2
    paired_block = 3
  []
  [diverging_mesh]
    type = FileMeshGenerator
    file = 'expansion_quad.e'
  []
[]

[Modules]
  [IncompressibleNavierStokes]
    equation_type = steady-state

    # no slip BCs
    velocity_boundary = 'bottom right left'
    velocity_function = '0 1    0 0   0 0'
    pressure_boundary = 'top'
    pressure_function = '1'

    density_name = rho
    dynamic_viscosity_name = mu

    integrate_p_by_parts = false
    order = SECOND
  []
[]

[Materials]
  [const]
    type = GenericConstantMaterial
    block = '1 2 3'
    prop_names = 'rho mu'
    prop_values = '1  1'
  []
  [ADconst]
    type = ADGenericFunctorMaterial
    block = '1 2 3'
    prop_names = 'rho_ad'
    prop_values = '1'
  []
[]

[Preconditioning]
  [SMP_PJFNK]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-ksp_gmres_restart -pc_type -sub_pc_type -sub_pc_factor_levels'
  petsc_options_value = '300                bjacobi  ilu          4'
  line_search = none
  nl_rel_tol = 1e-12
  nl_max_its = 6
  l_tol = 1e-6
  l_max_its = 300
[]

[Postprocessors]
  [inlet_mass_constant]
    type = VolumetricFlowRate
    boundary = bottom
    vel_x = vel_x
    vel_y = vel_y
    advected_variable = 1
  []
  [inlet_mass_matprop]
    type = VolumetricFlowRate
    boundary = bottom
    vel_x = vel_x
    vel_y = vel_y
    advected_mat_prop = 'rho_ad'
  []
  [mid1_mass]
    type = VolumetricFlowRate
    boundary = internal_bot
    vel_x = vel_x
    vel_y = vel_y
  []
  [other_mid1_mass]
    type = VolumetricFlowRate
    boundary = internal_bot
    vel_x = vel_x
    vel_y = vel_y
    advected_mat_prop = 'rho_ad'
  []
  [mid2_mass]
    type = VolumetricFlowRate
    boundary = internal_top
    vel_x = vel_x
    vel_y = vel_y
  []
  [outlet_mass]
    type = VolumetricFlowRate
    boundary = top
    vel_x = vel_x
    vel_y = vel_y
  []

  [inlet_momentum_x]
    type = VolumetricFlowRate
    boundary = bottom
    vel_x = vel_x
    vel_y = vel_y
    advected_variable = vel_x
  []
  [mid1_momentum_x]
    type = VolumetricFlowRate
    boundary = internal_bot
    vel_x = vel_x
    vel_y = vel_y
    advected_variable = vel_x
  []
  [mid2_momentum_x]
    type = VolumetricFlowRate
    boundary = internal_top
    vel_x = vel_x
    vel_y = vel_y
    advected_variable = vel_x
  []
  [outlet_momentum_x]
    type = VolumetricFlowRate
    boundary = top
    vel_x = vel_x
    vel_y = vel_y
    advected_variable = vel_x
  []

  [inlet_momentum_y]
    type = VolumetricFlowRate
    boundary = bottom
    vel_x = vel_x
    vel_y = vel_y
    advected_variable = vel_y
  []
  [mid1_momentum_y]
    type = VolumetricFlowRate
    boundary = internal_bot
    vel_x = vel_x
    vel_y = vel_y
    advected_variable = vel_y
  []
  [mid2_momentum_y]
    type = VolumetricFlowRate
    boundary = internal_top
    vel_x = vel_x
    vel_y = vel_y
    advected_variable = vel_y
  []
  [outlet_momentum_y]
    type = VolumetricFlowRate
    boundary = top
    vel_x = vel_x
    vel_y = vel_y
    advected_variable = vel_y
  []
[]

[Outputs]
  exodus = false
  csv = true
  inactive = 'console_mass console_momentum_x console_momentum_y'
  [console_mass]
    type = Console
    start_step = 1
    show = 'inlet_mass_variable inlet_mass_constant inlet_mass_matprop mid1_mass mid2_mass outlet_mass'
  []
  [console_momentum_x]
    type = Console
    start_step = 1
    show = 'inlet_momentum_x mid1_momentum_x mid2_momentum_x outlet_momentum_x'
  []
  [console_momentum_y]
    type = Console
    start_step = 1
    show = 'inlet_momentum_y mid1_momentum_y mid2_momentum_y outlet_momentum_y'
  []
[]
