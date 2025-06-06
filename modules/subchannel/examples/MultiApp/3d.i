# Following Advanced Burner Test Reactor Preconceptual Design Report
# Available at: https://www.ne.anl.gov/eda/ABTR_1cv2_ws.pdf

###################################################
# Geometric parameters
###################################################

# units are cm - do not forget to convert to meter
scale_factor = 0.01
fuel_element_pitch = ${fparse 14.598*scale_factor}
inter_assembly_gap = ${fparse 0.4*scale_factor}
duct_thickness = ${fparse 0.3*scale_factor}
fuel_pin_pitch = ${fparse 0.904*scale_factor}
fuel_pin_diameter= ${fparse 0.8*scale_factor}
# wire_z_spacing = ${fparse 20.32*scale_factor}
# wire_diameter = ${fparse 0.103*scale_factor}

# Reduced height for convenience
length_entry_fuel = '${fparse 20*scale_factor}'
length_heated_fuel = '${fparse 40*scale_factor}'
length_outlet_fuel = '${fparse 20*scale_factor}'
height = ${fparse length_entry_fuel+length_heated_fuel+length_outlet_fuel}
# orifice_plate_height = ${fparse 5*scale_factor}
duct_outside = ${fparse fuel_element_pitch - inter_assembly_gap}
duct_inside = ${fparse duct_outside - 2 * duct_thickness}
n_rings = 9

n_cells = 100

[Mesh]
  [subchannel]
    type = SCMDetailedTriSubChannelMeshGenerator
    nrings = '${fparse n_rings}'
    n_cells = ${n_cells}
    flat_to_flat = '${fparse duct_inside}'
    unheated_length_entry = '${fparse length_entry_fuel}'
    heated_length = '${fparse length_heated_fuel}'
    unheated_length_exit = '${fparse length_outlet_fuel}'
    pin_diameter = '${fparse fuel_pin_diameter}'
    pitch = '${fparse fuel_pin_pitch}'
  []

  [fuel_pins]
    type = SCMDetailedTriPinMeshGenerator
    input = subchannel
    nrings = '${fparse n_rings}'
    n_cells = ${n_cells}
    unheated_length_entry = '${fparse length_entry_fuel}'
    heated_length = '${fparse length_heated_fuel}'
    unheated_length_exit = '${fparse length_outlet_fuel}'
    pitch = '${fparse fuel_pin_pitch}'
    pin_diameter = '${fparse fuel_pin_diameter}'
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
  exodus = true
  csv = true
[]

[VectorPostprocessors]
  # These samplers are used for testing purposes.
  # Sampling over all nodes would creates megabytes of data
  # Ordering the samples by z-coordinate is consistent in parallel
  [sample_channel_center]
    type = LineValueSampler
    variable = 'mdot SumWij P DP h T rho mu S w_perim'
    sort_by = 'z'
    # n_nodes is n_elems + 1
    num_points = ${fparse n_cells + 1}
    start_point = '0 ${fuel_pin_pitch} 0'
    end_point = '0 ${fuel_pin_pitch} ${height}'
  []
  [sample_channel_edge]
    type = LineValueSampler
    variable = 'mdot SumWij P DP h T rho mu S w_perim'
    sort_by = 'z'
    num_points = ${fparse n_cells + 1}
    # the offset places the sample inside the channel
    start_point = '${fparse fuel_pin_pitch / 2} ${fparse (fuel_element_pitch - inter_assembly_gap - 2 * duct_thickness) / 2 - 1e-3} 0'
    end_point = '${fparse fuel_pin_pitch / 2} ${fparse (fuel_element_pitch - inter_assembly_gap - 2 * duct_thickness) / 2 - 1e-3} ${fparse height}'
  []
  [sample_channel_corner]
    type = LineValueSampler
    variable = 'mdot SumWij P DP h T rho mu S w_perim'
    sort_by = 'z'
    num_points = ${fparse n_cells + 1}
    start_point = '${fparse (n_rings - 0.5) * fuel_pin_pitch} 0 0'
    end_point = '${fparse (n_rings - 0.5) * fuel_pin_pitch} 0 ${height}'
  []
  [sample_pin_center]
    type = LineValueSampler
    variable = 'q_prime Tpin Dpin'
    sort_by = 'z'
    num_points = ${fparse n_cells + 1}
    start_point = '0 0 0'
    end_point = '0 0 ${height}'
  []
[]

[Executioner]
  type = Transient
[]
