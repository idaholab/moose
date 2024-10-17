# Following Advanced Burner Test Reactor Preconceptual Design Report
# Vailable at: https://www.ne.anl.gov/eda/ABTR_1cv2_ws.pdf

###################################################
# Geometric parameters
###################################################
f = ${fparse sqrt(3) / 2}

# units are cm - do not forget to convert to meter
scale_factor = 0.01
fuel_element_pitch = ${fparse 14.598*scale_factor}
inter_assembly_gap = ${fparse 0.4*scale_factor}
duct_thickness = ${fparse 0.3*scale_factor}
fuel_pin_pitch = ${fparse 0.904*scale_factor}
fuel_pin_diameter= ${fparse 0.8*scale_factor}
wire_z_spacing = ${fparse 20.32*scale_factor}
wire_diameter = ${fparse 0.103*scale_factor}
length_entry_fuel = ${fparse 60*scale_factor}
length_heated_fuel = ${fparse 80*scale_factor}
length_outlet_fuel = ${fparse 120*scale_factor}
height = ${fparse length_entry_fuel+length_heated_fuel+length_outlet_fuel}
orifice_plate_height = ${fparse 5*scale_factor}
duct_outside = ${fparse fuel_element_pitch - inter_assembly_gap}
duct_inside = ${fparse duct_outside - 2 * duct_thickness}
n_rings = 9

[Mesh]
  [subchannel]
    type = DetailedTriSubChannelMeshGenerator
    nrings = '${fparse n_rings}'
    n_cells = 100
    flat_to_flat = '${fparse duct_inside}'
    unheated_length_entry = '${fparse length_entry_fuel}'
    heated_length = '${fparse length_heated_fuel}'
    unheated_length_exit = '${fparse length_outlet_fuel}'
    pin_diameter = '${fparse fuel_pin_diameter}'
    pitch = '${fparse fuel_pin_pitch}'
    spacer_z = '${fparse orifice_plate_height} ${fparse length_entry_fuel}'
    spacer_k = '0.5 0.5'
  []

  [fuel_pins]
    type = DetailedTriPinMeshGenerator
    input = subchannel
    nrings = '${fparse n_rings}'
    n_cells = 100
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
[]

[Executioner]
  type = Transient
  end_time = 10
  dt = 1
[]
