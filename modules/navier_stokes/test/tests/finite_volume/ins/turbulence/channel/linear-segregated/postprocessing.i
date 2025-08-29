[Outputs]
  csv = true
[]

[AuxVariables]
  [pressure_over_density]
    type = MooseLinearVariableFVReal
    solver_sys = TKE_system
    initial_condition = ${k_init}
  []
[]

[AuxKernels]
  [compute_pressure_over_density]
    type = ParsedAux
    variable = pressure_over_density
    coupled_variables = 'pressure'
    expression = 'pressure/${rho}'
  []
[]

[VectorPostprocessors]
  [side_bottom]
    type = SideValueSampler
    boundary = 'bottom'
    variable = ${variables_to_sample}
    sort_by = 'x'
    execute_on = 'timestep_end'
  []
  [side_top]
    type = SideValueSampler
    boundary = 'top'
    variable = ${variables_to_sample}
    sort_by = 'x'
    execute_on = 'timestep_end'
  []
  [line_center_channel]
    type = LineValueSampler
    start_point = '${fparse 0.125 * L} ${fparse 0.0001} 0'
    end_point = '${fparse 0.875 * L} ${fparse 0.0001} 0'
    num_points = ${Mesh/block_1/nx}
    variable = ${variables_to_sample}
    sort_by = 'x'
    execute_on = 'timestep_end'
  []
  [line_quarter_radius_channel]
    type = LineValueSampler
    start_point = '${fparse 0.125 * L} ${fparse 0.5 * H} 0'
    end_point = '${fparse 0.875 * L} ${fparse 0.5 * H} 0'
    num_points =  ${Mesh/block_1/nx}
    variable = ${variables_to_sample}
    sort_by = 'x'
    execute_on = 'timestep_end'
  []
[]
