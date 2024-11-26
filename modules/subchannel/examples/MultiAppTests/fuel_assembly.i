# Following Advanced Burner Test Reactor Preconceptual Design Report
# Vailable at: https://www.ne.anl.gov/eda/ABTR_1cv2_ws.pdf
###################################################
# Thermal-hydraulics parameters
###################################################
T_in = 628.15
P_out = 758423 # Pa
reactor_power = 250e6 #WTh
fuel_assemblies_per_power_unit = '${fparse 2.5}'
fuel_pins_per_assembly = 217
pin_power = '${fparse reactor_power/(fuel_assemblies_per_power_unit*fuel_pins_per_assembly)}' # Approx.
mass_flux_in = '${fparse 2786}' # kg/(m2.s)

###################################################
# Geometric parameters
###################################################
f = '${fparse sqrt(3) / 2}'

# units are cm - do not forget to convert to meter
scale_factor = 0.01
fuel_element_pitch = '${fparse 14.598*scale_factor}'
inter_assembly_gap = '${fparse 0.4*scale_factor}'
duct_thickness = '${fparse 0.3*scale_factor}'
fuel_pin_pitch = '${fparse 0.904*scale_factor}'
fuel_pin_diameter = '${fparse 0.8*scale_factor}'
wire_z_spacing = '${fparse 20.32*scale_factor}'
wire_diameter = '${fparse 0.103*scale_factor}'
n_rings = 9
length_entry_fuel = '${fparse 60*scale_factor}'
length_heated_fuel = '${fparse 80*scale_factor}'
length_outlet_fuel = '${fparse 120*scale_factor}'
height = '${fparse length_entry_fuel+length_heated_fuel+length_outlet_fuel}'
orifice_plate_height = '${fparse 5*scale_factor}'
duct_outside = '${fparse fuel_element_pitch - inter_assembly_gap}'
duct_inside = '${fparse duct_outside - 2 * duct_thickness}'
###################################################

[TriSubChannelMesh]
  [subchannel]
    type = TriSubChannelMeshGenerator
    nrings = '${fparse n_rings}'
    n_cells = 100
    flat_to_flat = '${fparse duct_inside}'
    unheated_length_entry = '${fparse length_entry_fuel}'
    heated_length = '${fparse length_heated_fuel}'
    unheated_length_exit = '${fparse length_outlet_fuel}'
    pin_diameter = '${fparse fuel_pin_diameter}'
    pitch = '${fparse fuel_pin_pitch}'
    dwire = '${fparse wire_diameter}'
    hwire = '${fparse wire_z_spacing}'
    spacer_z = '${fparse orifice_plate_height} ${fparse length_entry_fuel}'
    spacer_k = '0.5 0.5'
  []

  [fuel_pins]
    type = TriPinMeshGenerator
    input = subchannel
    nrings = '${fparse n_rings}'
    n_cells = 100
    unheated_length_entry = '${fparse length_entry_fuel}'
    heated_length = '${fparse length_heated_fuel}'
    unheated_length_exit = '${fparse length_outlet_fuel}'
    pitch = '${fparse fuel_pin_pitch}'
  []

  [duct]
    type = TriDuctMeshGenerator
    input = fuel_pins
    nrings = '${fparse n_rings}'
    n_cells = 100
    flat_to_flat = '${fparse duct_inside}'
    unheated_length_entry = '${fparse length_entry_fuel}'
    heated_length = '${fparse length_heated_fuel}'
    unheated_length_exit = '${fparse length_outlet_fuel}'
    pitch = '${fparse fuel_pin_pitch}'
  []
[]

[Functions]
  [axial_heat_rate]
    type = ParsedFunction
    value = 'if(z>l1 & z<l2, 1.0, 0.0)'
    #'(pi/2)*sin(pi*z/L)'
    vars = 'l1 l2'
    vals = '${length_entry_fuel} ${fparse length_entry_fuel+length_heated_fuel}'
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
  [Tpin]
    block = fuel_pins
  []
  [Dpin]
    block = fuel_pins
  []
  [rho]
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
  [mu]
    block = subchannel
  []
  [q_prime_duct]
    block = duct
    initial_condition = 0
  []
  [Tduct]
    block = duct
  []
[]

[FluidProperties]
  [sodium]
    type = PBSodiumFluidProperties
  []
[]

[Problem]
  type = TriSubChannel1PhaseProblem
  fp = sodium
  n_blocks = 50
  P_out = ${P_out}
  CT = 1.0
  enforce_uniform_pressure = false
  compute_density = false #true
  compute_viscosity = true #true
  compute_power = true #true
  P_tol = 1.0e-3
  T_tol = 1.0e-3
  implicit = false
  segregated = true
  staggered_pressure = false
  monolithic_thermal = false
  verbose_multiapps = true
  verbose_subchannel = false
[]

[ICs]
  [S_IC]
    type = SCMTriFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = SCMTriWettedPerimIC
    variable = w_perim
  []

  [q_prime_IC]
    type = SCMTriPowerIC
    variable = q_prime
    power = ${pin_power} # W
    filename = "pin_power_profile217.txt"
    axial_heat_rate = axial_heat_rate
  []

  [T_ic]
    type = ConstantIC
    variable = T
    value = ${T_in}
  []

  [Dpin_ic]
    type = ConstantIC
    variable = Dpin
    value = ${fuel_pin_diameter}
  []

  [P_ic]
    type = ConstantIC
    variable = P
    value = 0.0
  []

  [DP_ic]
    type = ConstantIC
    variable = DP
    value = 0.0
  []

  [Viscosity_ic]
    type = ViscosityIC
    variable = mu
    p = ${P_out}
    T = T
    fp = sodium
  []

  [rho_ic]
    type = RhoFromPressureTemperatureIC
    variable = rho
    p = ${P_out}
    T = T
    fp = sodium
  []

  [h_ic]
    type = SpecificEnthalpyFromPressureTemperatureIC
    variable = h
    p = ${P_out}
    T = T
    fp = sodium
  []

  [mdot_ic]
    type = ConstantIC
    variable = mdot
    value = 0.0
  []

  [T_duct_ic]
    type = ConstantIC
    variable = Tduct
    value = ${T_in}
  []
[]

[AuxKernels]
  [T_in_bc]
    type = ConstantAux
    variable = T
    boundary = inlet
    value = ${T_in}
    execute_on = 'timestep_begin'
    block = subchannel
  []
  [mdot_in_bc]
    type = SCMMassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    mass_flux = ${mass_flux_in}
    execute_on = 'timestep_begin'
    block = subchannel
  []
[]

[UserObjects]
  active = ''
  [Tpin_avg_uo]
    type = NearestPointLayeredAverage
    direction = z
    num_layers = 1000
    variable = Tpin
    block = fuel_pins
    points = '${fparse 0.012} 0.0 0.0'
    execute_on = timestep_end
  []
[]

[Outputs]
  exodus = true
[]

[Executioner]
  type = Transient
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

################################################################################
# A multiapp that projects data to a detailed mesh
################################################################################
# [MultiApps]
#   #active = ''
#   [viz]
#     type = TransientMultiApp
#     input_files = "3d.i"
#     execute_on = "final"
#   []
# []
#
# [Transfers]
#   #active = ''
#   [subchannel_transfer]
#     type = SCMSolutionTransfer
#     to_multi_app = viz
#     variable = 'mdot SumWij P DP h T rho mu S'
#   []
#   [pin_transfer]
#     type = SCMPinSolutionTransfer
#     to_multi_app = viz
#     variable = 'Tpin q_prime'
#   []
# []
