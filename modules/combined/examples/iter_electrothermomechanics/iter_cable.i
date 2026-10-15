# ITER Cable Mechanics: Full Cable with Jacket
# Geometry: Full cable (bundle + jacket, channel deleted)
# Materials: Cu-Nb3Sn effective material bundle + JK2LB steel jacket
# Use case: Complete ITER cable simulation with structural jacket

# Geometric parameters
steel_jacket_side_length = '${units 49 mm}'

# Cooling channel diameter is total 9 mm: 7 mm helium channel and 2 mm spiral tube
channel_radius = ${units 4.5 mm}

!include iter_cable.params

[Mesh]
  [circle]
    type = ConcentricCircleMeshGenerator
    num_sectors = '${fparse refinement_level * 10}'
    has_outer_square = true
    radii = '${channel_radius} ${cable_radius}'
    rings = '1 ${fparse refinement_level * 5} ${fparse refinement_level * 5}'
    pitch = ${steel_jacket_side_length}
    preserve_volumes = true
    smoothing_max_it = 3
  []
  [delete_channel]
    type = BlockDeletionGenerator
    input = circle
    block = '1'
  []
  # After deletion: block 2 (middle annulus) = bundle, block 3 (outer square) = jacket
  [name_blocks]
    type = RenameBlockGenerator
    input = delete_channel
    old_block = '2 3'
    new_block = 'bundle jacket'
  []
  [stabilizer]
    type = AdvancedExtruderGenerator
    input = name_blocks
    direction = '0 0 1'
    heights = '${cable_length}'
    num_layers = '${fparse refinement_level * 25}'
    biases = '1'
    bottom_boundary = 'axial_start'
    top_boundary = 'axial_end'
  []
  # Pin at jacket corners instead of edge midpoints to minimize constraint effects
  [pin_bottom_0_deg]
    type = ExtraNodesetGenerator
    input = stabilizer
    new_boundary = pin_bottom_0_deg
    coord = '${fparse steel_jacket_side_length / 2} ${fparse steel_jacket_side_length / 2} 0'
    use_closest_node = true
  []
  [pin_bottom_90_deg]
    type = ExtraNodesetGenerator
    input = pin_bottom_0_deg
    new_boundary = pin_bottom_90_deg
    coord = '${fparse -steel_jacket_side_length / 2} ${fparse steel_jacket_side_length / 2} 0'
    use_closest_node = true
  []
  [pin_bottom_180_deg]
    type = ExtraNodesetGenerator
    input = pin_bottom_90_deg
    new_boundary = pin_bottom_180_deg
    coord = '${fparse -steel_jacket_side_length / 2} ${fparse -steel_jacket_side_length / 2} 0'
    use_closest_node = true
  []
[]

!include cylindrical_base.i

[Materials]
  # Block-specific material overrides for multi-block geometry

  # Effective Cu-Nb3Sn properties for bundle: 2/3 Cu + 1/3 Nb3Sn
  [youngs_modulus_effective]
    type = ADParsedMaterial
    block = 'bundle'
    property_name = youngs_modulus
    material_property_names = 'youngs_modulus_cu youngs_modulus_nb3sn'
    expression = '(2.0/3.0)*youngs_modulus_cu + (1.0/3.0)*youngs_modulus_nb3sn'
  []

  [poissons_ratio_effective]
    type = ADParsedMaterial
    block = 'bundle'
    property_name = poissons_ratio
    material_property_names = 'poissons_ratio_cu poissons_ratio_nb3sn'
    expression = '(2.0/3.0)*poissons_ratio_cu + (1.0/3.0)*poissons_ratio_nb3sn'
  []

  # Steel (JK2LB) properties for jacket - placeholders
  [youngs_modulus_steel]
    type = ADGenericConstantMaterial
    block = 'jacket'
    prop_names = 'youngs_modulus'
    prop_values = '200e3' # Placeholder: 200 GPa in MPa
  []

  [poissons_ratio_steel]
    type = ADGenericConstantMaterial
    block = 'jacket'
    prop_names = 'poissons_ratio'
    prop_values = '0.3' # Placeholder
  []

  # Thermal expansion materials for each block

  # Effective thermal expansion for bundle
  [expansion_effective]
    type = ADComputeInstantaneousThermalExpansionFunctionEigenstrain
    block = 'bundle'
    temperature = T
    thermal_expansion_function = thermal_expansion_func_effective
    stress_free_temperature = ${stress_free_temperature}
    eigenstrain_name = thermal_expansion
    outputs = 'exodus'
  []

  # Steel thermal expansion for jacket
  [expansion_steel]
    type = ADComputeInstantaneousThermalExpansionFunctionEigenstrain
    block = 'jacket'
    temperature = T
    thermal_expansion_function = thermal_expansion_func_steel
    stress_free_temperature = ${stress_free_temperature}
    eigenstrain_name = thermal_expansion
    outputs = 'exodus'
  []
[]

[Functions]
  # Steel thermal expansion (placeholder: constant)
  [thermal_expansion_func_steel]
    type = ParsedFunction
    expression = '1.5e-5' # Placeholder: constant 1.5e-5 / K
  []
[]

[Postprocessors]
  # Bundle-specific averages
  [stress_xx_bundle]
    type = ElementAverageValue
    block = 'bundle'
    variable = stress_xx
    use_displaced_mesh = true
    execute_on = 'INITIAL TIMESTEP_END'
    outputs = 'csv'
  []
  [stress_zz_bundle]
    type = ElementAverageValue
    block = 'bundle'
    variable = stress_zz
    use_displaced_mesh = true
    execute_on = 'INITIAL TIMESTEP_END'
    outputs = 'csv'
  []
  [stress_xy_bundle]
    type = ElementAverageValue
    block = 'bundle'
    variable = stress_xy
    use_displaced_mesh = true
    execute_on = 'INITIAL TIMESTEP_END'
    outputs = 'csv'
  []
  [vonmises_stress_bundle]
    type = ElementAverageValue
    block = 'bundle'
    variable = vonmises_stress
    use_displaced_mesh = true
    execute_on = 'INITIAL TIMESTEP_END'
    outputs = 'csv'
  []

  # Jacket-specific averages
  [stress_xx_jacket]
    type = ElementAverageValue
    block = 'jacket'
    variable = stress_xx
    use_displaced_mesh = true
    execute_on = 'INITIAL TIMESTEP_END'
    outputs = 'csv'
  []
  [stress_zz_jacket]
    type = ElementAverageValue
    block = 'jacket'
    variable = stress_zz
    use_displaced_mesh = true
    execute_on = 'INITIAL TIMESTEP_END'
    outputs = 'csv'
  []
  [stress_xy_jacket]
    type = ElementAverageValue
    block = 'jacket'
    variable = stress_xy
    use_displaced_mesh = true
    execute_on = 'INITIAL TIMESTEP_END'
    outputs = 'csv'
  []
  [vonmises_stress_jacket]
    type = ElementAverageValue
    block = 'jacket'
    variable = vonmises_stress
    use_displaced_mesh = true
    execute_on = 'INITIAL TIMESTEP_END'
    outputs = 'csv'
  []
[]

[VectorPostprocessors]
  # Sample Lorentz force through bundle and jacket - start slightly inside bundle (channel deleted), extend to jacket edge
  [line_sample_0deg]
    type = LineValueSampler
    start_point = '${fparse channel_radius * 1.05} 0 ${fparse cable_length / 2}'
    end_point = '${fparse steel_jacket_side_length / 2} 0 ${fparse cable_length / 2}'
    num_points = 50
    sort_by = x
    use_displaced_mesh = true
    variable = 'lorentz_x_aux lorentz_y_aux lorentz_z_aux'
  []
  [line_sample_45deg]
    type = LineValueSampler
    start_point = '${fparse channel_radius * 1.05 / sqrt(2)} ${fparse channel_radius * 1.05 / sqrt(2)} ${fparse cable_length / 2}'
    end_point = '${fparse steel_jacket_side_length / (2 * sqrt(2))} ${fparse steel_jacket_side_length / (2 * sqrt(2))} ${fparse cable_length / 2}'
    num_points = 50
    sort_by = x
    use_displaced_mesh = true
    variable = 'lorentz_x_aux lorentz_y_aux lorentz_z_aux'
  []
  [line_sample_90deg]
    type = LineValueSampler
    start_point = '0 ${fparse channel_radius * 1.05} ${fparse cable_length / 2}'
    end_point = '0 ${fparse steel_jacket_side_length / 2} ${fparse cable_length / 2}'
    num_points = 50
    sort_by = y
    use_displaced_mesh = true
    variable = 'lorentz_x_aux lorentz_y_aux lorentz_z_aux'
  []
  # Sample along axial direction at constant radius in bundle for uniformity check
  # [line_sample_axial]
  #   type = LineValueSampler
  #   start_point = '${fparse cable_radius * 0.8} 0 0'
  #   end_point = '${fparse cable_radius * 0.8} 0 ${cable_length}'
  #   num_points = 50
  #   sort_by = z
  #   use_displaced_mesh = true
  #   variable = 'lorentz_x_aux lorentz_y_aux lorentz_z_aux'
  # []
[]

[Outputs]
  [exodus]
    type = Exodus
    file_base = 'data/cable/iter_cable_out'
  []
  [csv]
    type = CSV
    file_base = 'data/cable/iter_cable_out'
  []
[]
