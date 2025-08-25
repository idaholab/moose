[GlobalParams]
    nx = 2
    ny = 2
    n_cells = 10
    pitch = 0.014605
    pin_diameter = 0.012065
    side_gap = 0.0015875
    heated_length = 1.0
[]

[Mesh]
    [subchannel]
        type = SCMDetailedQuadSubChannelMeshGenerator
    []
    [fuel_pins]
        type = SCMDetailedQuadPinMeshGenerator
        input = subchannel
    []
[]

[AuxVariables]
    [mdot]
        block = subchannel
    []
    [SumWij]
        block = subchannel
    []
    [P]
        block = subchannel
    []
    [DP]
        block = subchannel
    []
    [h]
        block = subchannel
    []
    [T]
        block = subchannel
    []
    [rho]
        block = subchannel
    []
    [mu]
        block = subchannel
    []
    [S]
        block = subchannel
    []
    [w_perim]
        block = subchannel
    []
    [q_prime]
        block = fuel_pins
    []
    [Tpin]
        block = fuel_pins
    []
    [Dpin]
        block = fuel_pins
    []
[]

[Problem]
    type = NoSolveProblem
[]

[Outputs]
    csv = true
    exodus = true
[]

[Executioner]
    type = Steady
[]

[VectorPostprocessors]
  # These samplers are used for testing purposes.
  [sample_center_pin]
    type = LineValueSampler
    variable = 'q_prime Tpin Dpin'
    sort_by = 'z'
    num_points = 11
    start_point = '0 0 0'
    end_point = '0 0 1.0'
  []
[]
