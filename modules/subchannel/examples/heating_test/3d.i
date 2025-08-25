pitch = 0.0126
unheated_length_entry = 2.5
heated_length = 5.0
unheated_length_exit = 2.5
height = ${fparse unheated_length_entry + heated_length + unheated_length_exit}
n_cells = 20

[Mesh]
  [subchannel]
    type = SCMDetailedQuadSubChannelMeshGenerator
    nx = 3
    ny = 3
    n_cells = '${n_cells}'
    pitch = '${pitch}'
    pin_diameter = 0.00950
    side_gap = 0.00095
    unheated_length_entry = '${unheated_length_entry}'
    heated_length = '${heated_length}'
    unheated_length_exit = '${unheated_length_exit}'
  []
[]

[AuxVariables]
  [mdot]
  []
  [SumWij]
  []
  [P]
  []
  [DP]
  []
  [h]
  []
  [T]
  []
  [rho]
  []
  [mu]
  []
  [S]
  []
  [w_perim]
  []
  [q_prime]
  []
[]

[Problem]
  type = NoSolveProblem
[]

[Outputs]
  exodus = true
  csv = true
[]

[VectorPostprocessors]
  # These samplers are used for testing purposes.
  # Sampling over all nodes would creates megabytes of data
  # Ordering the samples by z-coordinate is consistent in parallel
  [sample_channel_edge]
    type = LineValueSampler
    variable = 'mdot SumWij P DP h T rho mu S w_perim'
    sort_by = 'z'
    num_points = ${fparse n_cells + 1}
    start_point = '${fparse pitch} 0 0'
    end_point = '${fparse pitch} 0  ${fparse height}'
  []
  [sample_channel_corner]
    type = LineValueSampler
    variable = 'mdot SumWij P DP h T rho mu S w_perim'
    sort_by = 'z'
    num_points = ${fparse n_cells + 1}
    start_point = '${fparse pitch} ${fparse pitch} 0'
    end_point = '${fparse pitch} ${fparse pitch}  ${fparse height}'
  []
[]

[Executioner]
  type = Steady
[]
