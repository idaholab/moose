# Following Advanced Burner Test Reactor Preconceptual Design Report
# Vailable at: https://www.ne.anl.gov/eda/ABTR_1cv2_ws.pdf
# This file creates the 3D mesh where subchannel projects to
###################################################
# Geometric parameters
##################################################
scale_factor = 0.01
fuel_pin_pitch = '${fparse 0.5664*scale_factor}'
fuel_pin_diameter = '${fparse 0.4419*scale_factor}'
inner_duct_in = '${fparse 4.64*scale_factor}'
n_rings = 5
heated_length = '${fparse 34.3*scale_factor}'
unheated_length_exit = '${fparse 26.9*scale_factor}'
###################################################

[TriSubChannelMesh]
  [subchannel]
    type = DetailedTriSubChannelMeshGenerator
    nrings = '${n_rings}'
    n_cells = 50
    flat_to_flat = '${inner_duct_in}'
    unheated_length_exit = '${unheated_length_exit}'
    heated_length = '${heated_length}'
    pin_diameter = '${fuel_pin_diameter}'
    pitch = '${fuel_pin_pitch}'
  []

  [fuel_pins]
    type = DetailedTriPinMeshGenerator
    input = subchannel
    nrings = '${n_rings}'
    n_cells = 50
    unheated_length_exit = '${unheated_length_exit}'
    heated_length = '${heated_length}'
    pin_diameter = '${fuel_pin_diameter}'
    pitch = '${fuel_pin_pitch}'
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
