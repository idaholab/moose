# Tests the ADElementIntegralMaterialPropertyRZ post-processor.

R_o = 0.2
thickness = 0.05
R_i = ${fparse R_o - thickness}

L = 3.0
V = ${fparse pi * (R_o^2 - R_i^2) * L}

rho_value = 5.0
mass = ${fparse rho_value * V}

[Materials]
  [hs_mat]
    type = ADGenericConstantMaterial
    prop_names = 'density specific_heat thermal_conductivity'
    prop_values = '${rho_value} 1.0 1.0'
  []
[]

[Components]
  [heat_structure]
    type = HeatStructureCylindrical

    position = '1 2 3'
    orientation = '1 1 1'
    inner_radius = ${R_i}
    length = ${L}
    n_elems = 50

    names = 'region1'
    widths = '${thickness}'
    n_part_elems = '5'

    initial_T = 300
  []
[]

[Postprocessors]
  [mass]
    type = ADElementIntegralMaterialPropertyRZ
    axis_point = '1 2 3'
    axis_dir = '1 1 1'
    mat_prop = density
    execute_on = 'INITIAL'
  []
  [mass_error]
    type = RelativeDifferencePostprocessor
    value1 = mass
    value2 = ${mass}
    execute_on = 'INITIAL'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  file_base = 'element_integral_material_property_rz'
  [csv]
    type = CSV
    show = 'mass_error'
    execute_on = 'INITIAL'
  []
[]
