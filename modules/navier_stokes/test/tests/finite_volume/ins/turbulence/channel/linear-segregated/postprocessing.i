
[Outputs]
  csv = true
[]

[VectorPostprocessors]
  [side_bottom]
    type = SideValueSampler
    boundary = 'bottom'
    variable = 'vel_x vel_y pressure TKE TKED'
    sort_by = 'x'
    execute_on = 'timestep_end'
  []
  [side_top]
    type = SideValueSampler
    boundary = 'top'
    variable = 'vel_x vel_y pressure TKE TKED'
    sort_by = 'x'
    execute_on = 'timestep_end'
  []
  [line_center_channel]
    type = LineValueSampler
    start_point = '${fparse 0.125 * L} ${fparse 0.0001} 0'
    end_point = '${fparse 0.875 * L} ${fparse 0.0001} 0'
    num_points = ${Mesh/block_1/nx}
    variable = 'vel_x vel_y pressure TKE TKED'
    sort_by = 'x'
    execute_on = 'timestep_end'
  []
  [line_quarter_radius_channel]
    type = LineValueSampler
    start_point = '${fparse 0.125 * L} ${fparse 0.5 * H} 0'
    end_point = '${fparse 0.875 * L} ${fparse 0.5 * H} 0'
    num_points =  ${Mesh/block_1/nx}
    variable = 'vel_x vel_y pressure TKE TKED'
    sort_by = 'x'
    execute_on = 'timestep_end'
  []
[]
