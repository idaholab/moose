# Tests the HeatRateConductionRZ post-processor.

R_i = 0.1
thickness = 0.2
L = 3.0

R_o = ${fparse R_i + thickness}
S = ${fparse 2 * pi * R_o * L}

k = 20.0
T_i = 300.0
T_o = 500.0

dT_dr = ${fparse (T_o - T_i) / thickness}

Q_exact = ${fparse k * dT_dr * S}

[Materials]
  [hs_mat]
    type = ADGenericConstantMaterial
    prop_names = 'density specific_heat thermal_conductivity'
    prop_values = '1.0 1.0 ${k}'
  []
[]

[Functions]
  [T_fn]
    type = ParsedFunction
    expression = '${T_i} + (y - ${R_i}) * ${dT_dr}'
  []
[]

[Components]
  [heat_structure]
    type = HeatStructureCylindrical

    position = '0 0 0'
    orientation = '1 0 0'
    inner_radius = ${R_i}
    length = ${L}
    n_elems = 50

    names = 'region1'
    widths = '${thickness}'
    n_part_elems = '5'

    initial_T = T_fn
  []
[]

[Postprocessors]
  [Q_pp]
    type = HeatRateConductionRZ
    boundary = heat_structure:outer
    axis_point = '0 0 0'
    axis_dir = '1 0 0'
    temperature = T_solid
    thermal_conductivity = thermal_conductivity
    inward = true
    execute_on = 'INITIAL'
  []
  [Q_err]
    type = RelativeDifferencePostprocessor
    value1 = Q_pp
    value2 = ${Q_exact}
    execute_on = 'INITIAL'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 0
[]

[Outputs]
  file_base = 'heat_rate_conduction_rz'
  [csv]
    type = CSV
    show = 'Q_err'
    execute_on = 'INITIAL'
  []
[]
