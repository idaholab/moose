# ==============================================================================
# Step6 Newton-style setup on the simplified step6s baffle geometry.
#
# The original step6newton.i uses several Pronghorn-only pebble-bed correlations.
# This input keeps the NavierStokesFV action setup and replaces those correlations
# with basic MOOSE functor materials so the case can run in this tree.
# ==============================================================================
outlet_pressure = 5.84e+6
T_inlet = 533.25
inlet_density = 5.291
pebble_diameter = 0.06
thermal_mass_scaling = 0.1

mass_flow_rate = 64.3
bed_radius = 1.2
flow_area = '${fparse pi * bed_radius * bed_radius}'
flow_vel = '${fparse mass_flow_rate / flow_area / inlet_density}'
inlet_u = '${fparse -flow_vel}'

power_fn_scaling = 0.9792628
offset = -1.45819

bottom_reflector_Dh = 0.1
c_drag_old = 10
bed_forch = 10.60
clean_region_forch = 0

[Mesh]
  block_id = '1 2 3 4 5 6'
  block_name = 'pebble_bed cavity bottom_reflector side_reflector bottom_plenum upper_plenum'

  [cartesian_mesh]
    type = CartesianMeshGenerator
    dim = 2

    dx = '0.20 0.20 0.20 0.20 0.20 0.20
          0.010 0.055
          0.13
          0.102 0.102 0.102
          0.120'

    ix = '1 1 1 1 1 1
          1 1
          1
          1 1 1
          1'

    dy = '0.100 0.100 0.967
          0.1709 0.1709 0.1709 0.1709 0.1709
          0.4465 0.4465 0.4465 0.4465 0.4465 0.4465 0.4465 0.4465 0.4465 0.4465
          0.4465 0.4465 0.4465 0.4465 0.4465 0.4465 0.4465 0.4465 0.4465 0.4465
          0.458 0.712'

    iy = '1 2 2
          2 2 1 1 1
          4 1 1 1 1 1 1 1 1 1
          1 1 1 1 1 1 1 1 1 4
          4 2'

    subdomain_id = '4 4 4 4 4 4 4 4 4 4 4 4 4
                    4 4 4 4 4 4 4 4 4 4 4 4 4
                    5 5 5 5 5 5 5 5 5 4 4 4 4
                    3 3 3 3 3 3 4 4 4 4 4 4 4
                    3 3 3 3 3 3 4 4 4 4 4 4 4
                    3 3 3 3 3 3 4 4 4 4 4 4 4
                    3 3 3 3 3 3 4 4 4 4 4 4 4
                    3 3 3 3 3 3 4 4 4 4 4 4 4
                    1 1 1 1 1 1 4 4 4 4 4 4 4
                    1 1 1 1 1 1 4 4 4 4 4 4 4
                    1 1 1 1 1 1 4 4 4 4 4 4 4
                    1 1 1 1 1 1 4 4 4 4 4 4 4
                    1 1 1 1 1 1 4 4 4 4 4 4 4
                    1 1 1 1 1 1 4 4 4 4 4 4 4
                    1 1 1 1 1 1 4 4 4 4 4 4 4
                    1 1 1 1 1 1 4 4 4 4 4 4 4
                    1 1 1 1 1 1 4 4 4 4 4 4 4
                    1 1 1 1 1 1 4 4 4 4 4 4 4
                    1 1 1 1 1 1 4 4 4 4 4 4 4
                    1 1 1 1 1 1 4 4 4 4 4 4 4
                    1 1 1 1 1 1 4 4 4 4 4 4 4
                    1 1 1 1 1 1 4 4 4 4 4 4 4
                    1 1 1 1 1 1 4 4 4 4 4 4 4
                    1 1 1 1 1 1 4 4 4 4 4 4 4
                    1 1 1 1 1 1 4 4 4 4 4 4 4
                    1 1 1 1 1 1 4 4 4 4 4 4 4
                    1 1 1 1 1 1 4 4 4 4 4 4 4
                    1 1 1 1 1 1 4 4 4 4 4 4 4
                    2 2 2 2 2 2 6 6 6 6 6 6 4
                    4 4 4 4 4 4 4 4 4 4 4 4 4'
  []

  [cavity_top]
    type = SideSetsAroundSubdomainGenerator
    input = cartesian_mesh
    block = 2
    normal = '0 1 0'
    new_boundary = cavity_top
  []

  [upper_plenum_top]
    type = SideSetsAroundSubdomainGenerator
    input = cavity_top
    block = 6
    normal = '0 1 0'
    new_boundary = upper_plenum_top
  []

  [upper_plenum_bottom]
    type = SideSetsAroundSubdomainGenerator
    input = upper_plenum_top
    block = 6
    normal = '0 -1 0'
    new_boundary = upper_plenum_bottom
  []

  [inlet]
    type = SideSetsAroundSubdomainGenerator
    input = upper_plenum_bottom
    block = 6
    normal = '1 0 0'
    new_boundary = inlet
  []

  [baffle_cav_up]
    type = SideSetsBetweenSubdomainsGenerator
    input = inlet
    primary_block = 2
    paired_block = 6
    new_boundary = baffle_cav_up
  []

  [side_reflector_bed]
    type = SideSetsBetweenSubdomainsGenerator
    input = baffle_cav_up
    primary_block = 1
    paired_block = 4
    new_boundary = side_reflector_bed
  []

  [bottom_plenum_bottom]
    type = SideSetsAroundSubdomainGenerator
    input = side_reflector_bed
    block = 5
    new_boundary = bottom_plenum_bottom
    normal = '0 -1 0'
  []

  [bottom_plenum_top]
    type = SideSetsBetweenSubdomainsGenerator
    input = bottom_plenum_bottom
    primary_block = 5
    paired_block = 4
    normal = '0 1 0'
    new_boundary = bottom_plenum_top
  []

  [outlet]
    type = SideSetsAroundSubdomainGenerator
    input = bottom_plenum_top
    block = 5
    normal = '1 0 0'
    new_boundary = outlet
  []

  [side_reflector_bottom_reflector]
    type = SideSetsBetweenSubdomainsGenerator
    input = outlet
    primary_block = 3
    paired_block = 4
    new_boundary = side_reflector_bottom_reflector
  []

  [baffle_cav_bed]
    type = SideSetsBetweenSubdomainsGenerator
    input = side_reflector_bottom_reflector
    primary_block = 1
    paired_block = 2
    new_boundary = baffle_cav_bed
  []

  [bed_br_baffle]
    type = SideSetsBetweenSubdomainsGenerator
    input = baffle_cav_bed
    primary_block = 1
    paired_block = 3
    new_boundary = bed_br_baffle
  []

  [br_bp_baffle]
    type = SideSetsBetweenSubdomainsGenerator
    input = bed_br_baffle
    primary_block = 3
    paired_block = 5
    new_boundary = br_bp_baffle
  []

  [BreakBoundary]
    type = BreakBoundaryOnSubdomainGenerator
    input = br_bp_baffle
    boundaries = 'left'
  []

  [DeleteBoundary]
    type = BoundaryDeletionGenerator
    input = BreakBoundary
    boundary_names = 'left right top bottom'
  []

  [RenameBoundaryGenerator]
    type = RenameBoundaryGenerator
    input = DeleteBoundary
    old_boundary = ' left_to_1 left_to_2 left_to_3 left_to_4 left_to_5 side_reflector_bed side_reflector_bottom_reflector bottom_plenum_bottom bottom_plenum_top cavity_top       upper_plenum_top upper_plenum_bottom'
    new_boundary = '     bed_left  bed_left  bed_left  solid_left bed_left  bed_right          bed_right                      horizontal_walls    horizontal_walls horizontal_walls horizontal_walls horizontal_walls'
  []

  coord_type = RZ
[]

[FluidProperties]
  [fluid_properties_obj]
    type = HeliumFluidProperties
  []
[]

[Functions]
  [heat_source_fn]
    type = ParsedFunction
    expression = '${power_fn_scaling} * (-1.0612e4 * pow(y+${offset}, 4) + 1.5963e5 * pow(y+${offset}, 3)
                   -6.2993e5 * pow(y+${offset}, 2) + 1.4199e6 * (y+${offset}) + 5.5402e4)'
  []
[]

[Variables]
  [T_solid]
    type = INSFVEnergyVariable
    initial_condition = ${T_inlet}
    block = 'pebble_bed
             bottom_reflector
             side_reflector
             upper_plenum
             bottom_plenum'
  []
[]

[FVKernels]
  [energy_storage]
    type = PINSFVEnergyTimeDerivative
    variable = T_solid
    rho = rho_s
    cp = cp_s
    is_solid = true
    scaling = ${thermal_mass_scaling}
    porosity = porosity
  []

  [solid_energy_diffusion]
    type = FVAnisotropicDiffusion
    variable = T_solid
    coeff = 'effective_thermal_conductivity'
  []

  [heating]
    type = FVBodyForce
    variable = T_solid
    function = heat_source_fn
    block = 'pebble_bed'
  []

  [convection_pebble_bed_fluid]
    type = PINSFVEnergyAmbientConvection
    variable = T_solid
    T_fluid = T_fluid
    T_solid = T_solid
    is_solid = true
    h_solid_fluid = 'alpha'
    block = 'pebble_bed bottom_reflector'
  []
[]

[Modules]
  [NavierStokesFV]
    compressibility = 'weakly-compressible'
    porous_medium_treatment = true
    add_energy_equation = true
    block = 'pebble_bed cavity bottom_reflector upper_plenum bottom_plenum'

    density = rho
    dynamic_viscosity = mu
    thermal_conductivity = kappa

    porosity = porosity
    porosity_interface_pressure_treatment = 'bernoulli'

    initial_velocity = '${inlet_u} 0 0'
    initial_pressure = 5.4e6
    initial_temperature = ${T_inlet}

    inlet_boundaries = inlet
    momentum_inlet_types = fixed-velocity
    momentum_inlet_functors = '${inlet_u} 0'
    energy_inlet_types = fixed-temperature
    energy_inlet_functors = '${T_inlet}'

    wall_boundaries = 'bed_left bed_right horizontal_walls'
    momentum_wall_types = 'slip slip slip'
    energy_wall_types = 'heatflux heatflux heatflux'
    energy_wall_functors = '0 0 0'

    outlet_boundaries = outlet
    momentum_outlet_types = fixed-pressure
    pressure_functors = ${outlet_pressure}

    friction_types = 'darcy forchheimer'
    friction_coeffs = 'Darcy_coefficient Forchheimer_coefficient'

    ambient_convection_blocks = 'pebble_bed bottom_reflector'
    ambient_convection_alpha = 'alpha'
    ambient_temperature = 'T_solid'
  []
[]

[FunctorMaterials]
  [fluid_props_to_mat_props]
    type = GeneralFunctorFluidProps
    fp = fluid_properties_obj
    porosity = porosity
    pressure = pressure
    T_fluid = ${T_inlet}
    speed = speed
    characteristic_length = characteristic_length
    block = 'pebble_bed cavity bottom_reflector upper_plenum bottom_plenum'
  []

  [graphite_bed]
    type = ADGenericFunctorMaterial
    prop_names = 'rho_s cp_s kappa_s'
    prop_values = '1780.0 1697 26'
    block = 'pebble_bed'
  []

  [graphite_side_reflector]
    type = ADGenericFunctorMaterial
    prop_names = 'rho_s cp_s kappa_s'
    prop_values = '1780.0 1697 26'
    block = 'side_reflector'
  []

  [graphite_bottom_reflector]
    type = ADGenericFunctorMaterial
    prop_names = 'rho_s cp_s kappa_s'
    prop_values = '1780.0 1697 18.2'
    block = 'bottom_reflector'
  []

  [graphite_plenums]
    type = ADGenericFunctorMaterial
    prop_names = 'rho_s cp_s kappa_s'
    prop_values = '1780.0 1697 20.8'
    block = 'upper_plenum bottom_plenum'
  []

  [drag_new_convention]
    type = ADParsedFunctorMaterial
    expression = '${c_drag_old} * rho_fluid / porosity / fluid_mu'
    property_name = c_drag
    functor_symbols = 'rho_fluid porosity fluid_mu'
    functor_names = 'rho porosity mu'
    block = 'cavity upper_plenum bottom_plenum'
  []

  [darcy_null]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'darcy_null'
    prop_values = '0 0 0'
  []

  [darcy_c_drag]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'darcy_c_drag'
    prop_values = 'c_drag c_drag c_drag'
    block = 'cavity upper_plenum bottom_plenum'
  []

  [darcy]
    type = ADPiecewiseByBlockVectorFunctorMaterial
    prop_name = 'Darcy_coefficient'
    subdomain_to_prop_value = 'pebble_bed       darcy_null
                               cavity           darcy_c_drag
                               bottom_reflector darcy_null
                               upper_plenum     darcy_c_drag
                               bottom_plenum    darcy_c_drag'
  []

  [forch_null]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'forch_null'
    prop_values = '0 0 0'
  []

  [forch_bed]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'forch_bed'
    prop_values = '${bed_forch} ${bed_forch} ${bed_forch}'
    block = 'pebble_bed'
  []

  [forch_clean]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'forch_clean'
    prop_values = '${clean_region_forch} ${clean_region_forch} ${clean_region_forch}'
    block = 'cavity upper_plenum bottom_plenum'
  []

  [forch_bottom_reflector]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'forch_bottom_reflector'
    prop_values = '270 0.027 270'
    block = 'bottom_reflector'
  []

  [forch]
    type = ADPiecewiseByBlockVectorFunctorMaterial
    prop_name = 'Forchheimer_coefficient'
    subdomain_to_prop_value = 'pebble_bed       forch_bed
                               cavity           forch_clean
                               bottom_reflector forch_bottom_reflector
                               upper_plenum     forch_clean
                               bottom_plenum    forch_clean'
  []

  [porosity_material]
    type = ADPiecewiseByBlockFunctorMaterial
    prop_name = porosity
    subdomain_to_prop_value = 'pebble_bed       0.39
                               cavity           1
                               bottom_reflector 0.3
                               side_reflector   0
                               upper_plenum     0.2
                               bottom_plenum    0.2'
  []

  [effective_solid_conductivity]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'effective_thermal_conductivity'
    prop_values = 'kappa_s kappa_s kappa_s'
    block = 'pebble_bed
             bottom_reflector
             side_reflector
             upper_plenum
             bottom_plenum'
  []

  [fluid_thermal_conductivity]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'kappa'
    prop_values = 'k k k'
    block = 'pebble_bed cavity bottom_reflector upper_plenum bottom_plenum'
  []

  [pebble_bed_alpha]
    type = ADGenericFunctorMaterial
    prop_names = 'alpha'
    prop_values = '1.55e5'
    block = 'pebble_bed'
  []

  [reflector_alpha]
    type = ADGenericFunctorMaterial
    prop_names = 'alpha'
    prop_values = '2e4'
    block = 'bottom_reflector'
  []

  [characteristic_length]
    type = PiecewiseByBlockFunctorMaterial
    prop_name = characteristic_length
    subdomain_to_prop_value = 'pebble_bed       ${pebble_diameter}
                               bottom_reflector ${bottom_reflector_Dh}'
  []
[]

[Executioner]
  type = Transient
  end_time = 5e5
  [TimeStepper]
    type = IterationAdaptiveDT
    iteration_window = 4
    optimal_iterations = 8
    cutback_factor = 0.8
    growth_factor = 1.6
    dt = 0.003
  []
  line_search = l2
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu NONZERO superlu_dist'
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-5
  nl_max_its = 30
  automatic_scaling = true
[]

[Postprocessors]
  [inlet_mfr]
    type = VolumetricFlowRate
    advected_quantity = rho
    vel_x = 'superficial_vel_x'
    vel_y = 'superficial_vel_y'
    boundary = 'inlet'
    rhie_chow_user_object = pins_rhie_chow_interpolator
  []

  [outlet_mfr]
    type = VolumetricFlowRate
    advected_quantity = rho
    vel_x = 'superficial_vel_x'
    vel_y = 'superficial_vel_y'
    boundary = 'outlet'
    rhie_chow_user_object = pins_rhie_chow_interpolator
  []

  [inlet_pressure]
    type = SideAverageValue
    variable = pressure
    boundary = inlet
    outputs = none
  []

  [outlet_pressure]
    type = SideAverageValue
    variable = pressure
    boundary = outlet
    outputs = none
  []

  [pressure_drop]
    type = ParsedPostprocessor
    pp_names = 'inlet_pressure outlet_pressure'
    expression = 'inlet_pressure - outlet_pressure'
  []

  [enthalpy_inlet]
    type = VolumetricFlowRate
    boundary = inlet
    vel_x = superficial_vel_x
    vel_y = superficial_vel_y
    rhie_chow_user_object = 'pins_rhie_chow_interpolator'
    advected_quantity = 'rho_cp_temp'
    advected_interp_method = 'upwind'
    outputs = none
  []

  [enthalpy_outlet]
    type = VolumetricFlowRate
    boundary = outlet
    vel_x = superficial_vel_x
    vel_y = superficial_vel_y
    rhie_chow_user_object = 'pins_rhie_chow_interpolator'
    advected_quantity = 'rho_cp_temp'
    advected_interp_method = 'upwind'
    outputs = none
  []

  [enthalpy_balance]
    type = ParsedPostprocessor
    pp_names = 'enthalpy_inlet enthalpy_outlet'
    expression = 'abs(enthalpy_outlet) - abs(enthalpy_inlet)'
  []

  [heat_source_integral]
    type = ElementIntegralFunctorPostprocessor
    functor = heat_source_fn
    block = pebble_bed
  []

  [mass_flux_weighted_Tf_out]
    type = MassFluxWeightedFlowRate
    vel_x = superficial_vel_x
    vel_y = superficial_vel_y
    density = rho
    rhie_chow_user_object = 'pins_rhie_chow_interpolator'
    boundary = outlet
    advected_quantity = T_fluid
  []
[]

[Outputs]
  exodus = true
[]
