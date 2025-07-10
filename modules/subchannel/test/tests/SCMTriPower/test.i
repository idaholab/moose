
num_cells = 15
length = 0.5

[TriSubChannelMesh]
  [subchannel]
    type = SCMTriSubChannelMeshGenerator
    nrings = 3
    n_cells = ${num_cells}
    flat_to_flat = 0.60
    unheated_length_entry = 0.5
    heated_length = ${length}
    unheated_length_exit = 0.5
    pin_diameter = 0.1
    pitch = 0.13
    dwire = 0.03
    hwire = 0.3
  []

  [fuel_pins]
    type = SCMTriPinMeshGenerator
    input = subchannel
    nrings = 3
    n_cells = ${num_cells}
    unheated_length_entry = 0.5
    heated_length = ${length}
    unheated_length_exit = 0.5
    pitch = 0.13
  []
[]

[AuxVariables]
  [q_prime_aux]
    block = fuel_pins
  []
  [q_prime_ic]
    block = fuel_pins
  []
[]

[Functions]
  [axial_heat_rate]
    type = ParsedFunction
    value = '(pi/2)*sin(pi*z/L)'
    vars = 'L'
    vals = '${length}'
  []
[]

[AuxKernels]
  [q_prime_IC]
    type = SCMTriPowerAux
    variable = q_prime_aux
    power = 5.0e5 # W
    filename = "pin_power_profile.txt" #type in name of file that describes radial power profile
    axial_heat_rate = axial_heat_rate
    execute_on = 'initial'
  []
[]

[ICs]
  [q_prime_IC]
    type = SCMTriPowerIC
    variable = q_prime_ic
    power = 5.0e5 # W
    filename = "pin_power_profile.txt"
    axial_heat_rate = axial_heat_rate
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [Total_power_IC]
    type = ElementIntegralVariablePostprocessor
    variable = q_prime_ic
    block = fuel_pins
  []
  [Total_power_Aux]
    type = ElementIntegralVariablePostprocessor
    variable = q_prime_aux
    block = fuel_pins
  []
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
