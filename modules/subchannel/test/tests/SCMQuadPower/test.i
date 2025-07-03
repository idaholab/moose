num_cells = 15

[QuadSubChannelMesh]
  [sub_channel]
    type = SCMQuadSubChannelMeshGenerator
    nx = 3
    ny = 3
    n_cells = ${num_cells}
    pitch = 0.25
    pin_diameter = 0.125
    gap = 0.1
    unheated_length_entry = 0.5
    heated_length = 0.5
    unheated_length_exit = 0.5
    spacer_k = '0.0'
    spacer_z = '0'
  []
[]

[AuxVariables]
  [q_prime_aux]
  []
  [q_prime_ic]
  []
[]

[AuxKernels]
  [q_prime_IC]
    type = SCMQuadPowerAux
    variable = q_prime_aux
    power = 3.44e6 # W
    filename = "power_profile.txt" #type in name of file that describes radial power profile
    execute_on = 'initial'
  []
[]

[ICs]
  [q_prime_IC]
    type = SCMQuadPowerIC
    variable = q_prime_ic
    power = 3.44e6 # W
    filename = "power_profile.txt" #type in name of file that describes radial power profile
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[VectorPostprocessors]
  [line_check]
    type = LineValueSampler
    variable = 'q_prime_ic q_prime_aux'
    execute_on = 'TIMESTEP_END'
    sort_by = 'z'
    start_point = '0 0 0'
    end_point = '0 0 1.5'
    num_points = ${fparse num_cells + 1}
  []
[]

[Outputs]
  csv = true
[]
