# ITER Cable Mechanics: Cu-Nb3Sn Effective Cylinder
# Geometry: Single solid Cu-Nb3Sn cylinder (no channel, no jacket)
# Materials: Cu-Nb3Sn effective material (2/3 Cu + 1/3 Nb3Sn weighted average)
# Use case: Hybrid superconductor verification, no structural jacket effects

!include iter_cable.params

[Mesh]
  [circle]
    type = ConcentricCircleMeshGenerator
    num_sectors = '${fparse refinement_level * 10}'
    radii = '${cable_radius}'
    rings = '${fparse refinement_level * 5}'
    has_outer_square = false
    preserve_volumes = true
    smoothing_max_it = 3
  []
  [stabilizer]
    type = AdvancedExtruderGenerator
    input = circle
    direction = '0 0 1'
    heights = '${cable_length}'
    num_layers = '${fparse refinement_level * 25}'
    biases = '1'
    bottom_boundary = 'axial_start'
    top_boundary = 'axial_end'
  []
  [pin_bottom_0_deg]
    type = ExtraNodesetGenerator
    input = stabilizer
    new_boundary = pin_bottom_0_deg
    coord = '${cable_radius} 0 0'
    use_closest_node = true
  []
  [pin_bottom_90_deg]
    type = ExtraNodesetGenerator
    input = pin_bottom_0_deg
    new_boundary = pin_bottom_90_deg
    coord = '0 ${cable_radius} 0'
    use_closest_node = true
  []
  [pin_bottom_180_deg]
    type = ExtraNodesetGenerator
    input = pin_bottom_90_deg
    new_boundary = pin_bottom_180_deg
    coord = '${fparse -cable_radius} 0 0'
    use_closest_node = true
  []
[]

!include cylindrical_base.i

[Materials]
  # Effective Cu-Nb3Sn properties: 2/3 Cu + 1/3 Nb3Sn
  [youngs_modulus_effective]
    type = ADParsedMaterial
    property_name = youngs_modulus
    material_property_names = 'youngs_modulus_cu youngs_modulus_nb3sn'
    expression = '(2.0/3.0)*youngs_modulus_cu + (1.0/3.0)*youngs_modulus_nb3sn'
  []

  [poissons_ratio_effective]
    type = ADParsedMaterial
    property_name = poissons_ratio
    material_property_names = 'poissons_ratio_cu poissons_ratio_nb3sn'
    expression = '(2.0/3.0)*poissons_ratio_cu + (1.0/3.0)*poissons_ratio_nb3sn'
  []

  # Effective thermal expansion
  [expansion_effective]
    type = ADComputeInstantaneousThermalExpansionFunctionEigenstrain
    temperature = T
    thermal_expansion_function = thermal_expansion_func_effective
    stress_free_temperature = ${stress_free_temperature}
    eigenstrain_name = thermal_expansion
    outputs = 'exodus'
  []
[]

[VectorPostprocessors]
  # Sample Lorentz force along radial lines at different angles for axisymmetry check
  [line_sample_0deg]
    type = LineValueSampler
    start_point = '0 0 ${fparse cable_length / 2}'
    end_point = '${cable_radius} 0 ${fparse cable_length / 2}'
    num_points = 50
    sort_by = x
    use_displaced_mesh = true
    variable = 'lorentz_x_aux lorentz_y_aux lorentz_z_aux'
  []
  [line_sample_45deg]
    type = LineValueSampler
    start_point = '0 0 ${fparse cable_length / 2}'
    end_point = '${fparse cable_radius / sqrt(2)} ${fparse cable_radius / sqrt(2)} ${fparse cable_length / 2}'
    num_points = 50
    sort_by = x
    use_displaced_mesh = true
    variable = 'lorentz_x_aux lorentz_y_aux lorentz_z_aux'
  []
  [line_sample_90deg]
    type = LineValueSampler
    start_point = '0 0 ${fparse cable_length / 2}'
    end_point = '0 ${cable_radius} ${fparse cable_length / 2}'
    num_points = 50
    sort_by = y
    use_displaced_mesh = true
    variable = 'lorentz_x_aux lorentz_y_aux lorentz_z_aux'
  []
  # Sample along axial direction at constant radius for uniformity check
  [line_sample_axial]
    type = LineValueSampler
    start_point = '${fparse cable_radius * 0.8} 0 0'
    end_point = '${fparse cable_radius * 0.8} 0 ${cable_length}'
    num_points = 50
    sort_by = z
    use_displaced_mesh = true
    variable = 'lorentz_x_aux lorentz_y_aux lorentz_z_aux'
  []
[]

[Outputs]
  [exodus]
    type = Exodus
    file_base = 'data/cylinder/cylinder_out'
  []
  [csv]
    type = CSV
    file_base = 'data/cylinder/cylinder_out'
  []
[]
