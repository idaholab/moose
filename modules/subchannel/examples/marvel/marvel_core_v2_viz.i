heated_length = 0.51
unheated_length_entry = 0.2
unheated_length_exit = 0.2
[TriSubChannelMesh]
    [subchannel]
        type = DetailedTriSubChannelMeshGenerator
        nrings = 4
        n_cells = 60
        flat_to_flat = 0.22
        heated_length = ${heated_length}
        unheated_length_entry = ${unheated_length_entry}
        unheated_length_exit = ${unheated_length_exit}
        pin_diameter = 0.03269
        pitch = 0.0346514
    []

    [fuel_pins]
        type = DetailedTriPinMeshGenerator
        input = subchannel
        nrings = 4
        n_cells = 60
        heated_length = ${heated_length}
        unheated_length_entry = ${unheated_length_entry}
        unheated_length_exit = ${unheated_length_exit}
        pin_diameter = 0.03269
        pitch = 0.0346514
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
    [displacement]
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
    exodus = true
[]

[Executioner]
    type = Steady
[]
