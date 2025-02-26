mu = 1.1
rho = 1.1

# No scalar diffusion, as we are looking at advective flow rate for conservation
diff = 0

advected_interp_method = 'upwind'

[Problem]
  error_on_jacobian_nonzero_reallocation = true
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 5
    ymin = 0
    ymax = 1
    nx = 20
    ny = 10
  []
  [second_block]
    type = ParsedSubdomainMeshGenerator
    input = 'gen'
    combinatorial_geometry = 'x > 2.5001'
    block_id = '1'
  []
  [middle]
    type = SideSetsBetweenSubdomainsGenerator
    input = 'second_block'
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'mid'
  []
[]

[Physics]
  [NavierStokes]
    [Flow]
      [flow]
        compressibility = 'incompressible'
        porous_medium_treatment = true

        density = 'rho'
        dynamic_viscosity = 'mu'

        # Porosity settings
        porosity = 'porosity'
        porosity_interface_pressure_treatment = 'bernoulli'

        initial_velocity = '1 1e-6 0'
        initial_pressure = 0.0

        inlet_boundaries = 'left'
        momentum_inlet_types = 'fixed-velocity'
        momentum_inlet_functors = '1 0'
        wall_boundaries = 'top bottom'
        momentum_wall_types = 'slip slip'
        outlet_boundaries = 'right'
        momentum_outlet_types = 'fixed-pressure'
        pressure_functors = '0'

        mass_advection_interpolation = '${advected_interp_method}'
        momentum_advection_interpolation = '${advected_interp_method}'
      []
    []
    [ScalarTransport]
      [flow]
        passive_scalar_names = 'scalar'

        passive_scalar_diffusivity = '${diff}'

        # Should add +1.25 to scalar concentration on left domain
        # Then multiply by 2: 3.25 -> 6.5
        # then +2.5 on the right domain -> outlet flow at 9
        passive_scalar_coupled_source = Q
        passive_scalar_coupled_source_coeff = 0.1

        # See flow for inlet boundary
        passive_scalar_inlet_types = 'fixed-value'
        passive_scalar_inlet_functors = '2'

        # No need to specify wall or outlet boundaries here

        passive_scalar_advection_interpolation = '${advected_interp_method}'
      []
    []
  []
[]

[FunctorMaterials]
  [const]
    type = ADGenericFunctorMaterial
    prop_names = 'rho mu Q'
    prop_values = '${rho} ${mu} 10'
  []
  [eps_step]
    type = PiecewiseByBlockFunctorMaterial
    prop_name = 'porosity'
    subdomain_to_prop_value = '0 0.5
                               1 1'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_rel_tol = 1e-11
  nl_abs_tol = 1e-14
[]

# Some basic Postprocessors to visually examine the solution
[Postprocessors]
  [inlet-u]
    type = VolumetricFlowRate
    vel_x = 'superficial_vel_x'
    vel_y = 'superficial_vel_y'
    advected_quantity = '1'
    rhie_chow_user_object = 'pins_rhie_chow_interpolator'
    advected_interp_method = '${advected_interp_method}'
    subtract_mesh_velocity = 'false'
    boundary = 'left'
  []
  [outlet-u]
    type = VolumetricFlowRate
    vel_x = 'superficial_vel_x'
    vel_y = 'superficial_vel_y'
    advected_quantity = '1'
    rhie_chow_user_object = 'pins_rhie_chow_interpolator'
    advected_interp_method = '${advected_interp_method}'
    subtract_mesh_velocity = 'false'
    boundary = 'right'
  []
  [inlet-momx]
    type = VolumetricFlowRate
    vel_x = 'superficial_vel_x'
    vel_y = 'superficial_vel_y'
    advected_quantity = 'superficial_vel_x'
    rhie_chow_user_object = 'pins_rhie_chow_interpolator'
    advected_interp_method = '${advected_interp_method}'
    subtract_mesh_velocity = 'false'
    boundary = 'left'
  []
  [outlet-momx]
    type = VolumetricFlowRate
    vel_x = 'superficial_vel_x'
    vel_y = 'superficial_vel_y'
    advected_quantity = 'superficial_vel_x'
    rhie_chow_user_object = 'pins_rhie_chow_interpolator'
    advected_interp_method = '${advected_interp_method}'
    subtract_mesh_velocity = 'false'
    boundary = 'right'
  []
  [inlet-scalar]
    type = VolumetricFlowRate
    vel_x = 'superficial_vel_x'
    vel_y = 'superficial_vel_y'
    advected_quantity = 'scalar'
    rhie_chow_user_object = 'pins_rhie_chow_interpolator'
    advected_interp_method = '${advected_interp_method}'
    subtract_mesh_velocity = 'false'
    boundary = 'left'
  []
  [mid-scalar]
    type = VolumetricFlowRate
    vel_x = 'superficial_vel_x'
    vel_y = 'superficial_vel_y'
    advected_quantity = 'scalar'
    rhie_chow_user_object = 'pins_rhie_chow_interpolator'
    advected_interp_method = '${advected_interp_method}'
    subtract_mesh_velocity = 'false'
    boundary = 'mid'
  []
  [outlet-scalar]
    type = VolumetricFlowRate
    vel_x = 'superficial_vel_x'
    vel_y = 'superficial_vel_y'
    advected_quantity = 'scalar'
    rhie_chow_user_object = 'pins_rhie_chow_interpolator'
    advected_interp_method = '${advected_interp_method}'
    subtract_mesh_velocity = 'false'
    boundary = 'right'
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
