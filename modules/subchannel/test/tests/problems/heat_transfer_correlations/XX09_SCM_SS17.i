# Following Benchmark Specifications and Data Requirements for EBR-II Shutdown Heat Removal Tests SHRT-17 and SHRT-45R
# Available at: https://publications.anl.gov/anlpubs/2012/06/73647.pdf
###################################################
# Steady state subchannel calcultion
# Thermal-hydraulics parameters
###################################################
T_in = 624.70556 #Kelvin
Total_Surface_Area = 0.000854322 #m2
Mass_In = 2.45 #kg/sec
mass_flux_in = '${fparse Mass_In / Total_Surface_Area}' #kg/m2
P_out = 2.0e5 #Pa
Power_initial = 486200 #W (Page 26,35 of ANL document)
###################################################
# Geometric parameters
###################################################
scale_factor = 0.01
fuel_pin_pitch = '${fparse 0.5664*scale_factor}'
fuel_pin_diameter = '${fparse 0.4419*scale_factor}'
wire_z_spacing = '${fparse 15.24*scale_factor}'
wire_diameter = '${fparse 0.1244*scale_factor}'
inner_duct_in = '${fparse 4.64*scale_factor}'
n_rings = 5
heated_length = '${fparse 34.3*scale_factor}'
unheated_length_exit = '${fparse 26.9*scale_factor}'
###################################################

[TriSubChannelMesh]
  [subchannel]
    type = SCMTriSubChannelMeshGenerator
    nrings = ${n_rings}
    n_cells = 20
    flat_to_flat = ${inner_duct_in}
    unheated_length_exit = ${unheated_length_exit}
    heated_length = ${heated_length}
    pin_diameter = ${fuel_pin_diameter}
    pitch = ${fuel_pin_pitch}
    dwire = ${wire_diameter}
    hwire = ${wire_z_spacing}
  []

  [fuel_pins]
    type = SCMTriPinMeshGenerator
    input = subchannel
    nrings = ${n_rings}
    n_cells = 20
    unheated_length_exit = ${unheated_length_exit}
    heated_length = ${heated_length}
    pitch = ${fuel_pin_pitch}
  []

  [duct]
    type = SCMTriDuctMeshGenerator
    input = fuel_pins
    nrings = ${n_rings}
    n_cells = 20
    flat_to_flat = ${inner_duct_in}
    unheated_length_exit = ${unheated_length_exit}
    heated_length = ${heated_length}
    pitch = ${fuel_pin_pitch}
  []
[]

[Functions]
  [axial_heat_rate]
    type = ParsedFunction
    expression = '(pi/2)*sin(pi*z/L)*exp(-alpha*z)/(1.0/alpha*(1.0 - exp(-alpha*L)))*L'
    symbol_names = 'L alpha'
    symbol_values = '${heated_length} 1.8012'
  []
[]

[FluidProperties]
  [sodium]
    type = PBSodiumFluidProperties
  []
[]

[SubChannel]
  type = TriSubChannel1PhaseProblem
  fp = sodium
  n_blocks = 1
  P_out = ${P_out}
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_tol = 1.0e-4
  T_tol = 1.0e-5
  implicit = true
  segregated = false
  interpolation_scheme = 'upwind'
  verbose_subchannel = true
  friction_closure = 'cheng'
  mixing_closure = 'cheng_todreas'
[]

[SCMClosures]
  [cheng]
    type = SCMFrictionUpdatedChengTodreas
  []
  [dittus-boelter]
    type = SCMHTCDittusBoelter
  []
  [gnielinski]
    type = SCMHTCGnielinski
  []
  [kazimi-carelli]
    type = SCMHTCKazimiCarelli
  []
  [schad-modified]
    type = SCMHTCSchadModified
  []
  [graber-rieger]
    type = SCMHTCGraberRieger
  []
  [borishanskii]
    type = SCMHTCBorishanskii
  []
  [cheng_todreas]
    type = SCMMixingChengTodreas
    CT = 2.6
  []
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
    power = ${Power_initial}
    filename = "pin_power_profile61_uniform.txt"
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
  []
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  ### Central pin inlet temperature
  [Pin_Temp_0_Inlet]
    type = SCMPinSurfaceTemperature
    index = 0
    height = ${fparse heated_length*0.01}
  []
  ### Central pin center temperature
  [Pin_Temp_1_Center]
    type = SCMPinSurfaceTemperature
    index = 0
    height = ${fparse heated_length*0.5}
  []
  ### Central pin outlet temperature
  [Pin_Temp_2_Outlet]
    type = SCMPinSurfaceTemperature
    index = 0
    height = ${fparse heated_length*0.99}
  []
[]

[Executioner]
  type = Steady
[]
