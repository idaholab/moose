# This test is a simpified coupled case between the electromagnetic and
# heat transfer modules. While the file microwave_heating.i is a test
# utilizing the method of manufactured solutions, where both real and
# complex components of the electromagnetic properties are provided
# (such that no term is zeroed out), this test involves only the
# real components of the electromagnetic properties. In particular,
# this test supplies the fusing current to a copper wire and simulations
# the spatial and temporal heating profile until the wire reaches its
# melting point. The PDE's of this test file are as follows:
#
#   curl(curl(A)) + j*mu*omega*(sigma*A) = J
#   mag(E) = mag(-j*omega*A) + mag(J/sigma)
#   rho*C*dT/dt - div(k*grad(T)) = Q
#   Q = 0.5*sigma*mag(E)^2
#
# Where:
#   - A is the magnetic vector potential
#   - j is the sqrt(-1)
#   - mu is the permeability of free space
#   - omega is the angular frequency of the system
#   - sigma is the electric conductivity of the wire
#   - J is the supplied DC current
#   - E is the electric field
#   - rho is the density of copper
#   - C is the heat capacity of copper
#   - T is the temperature
#   - k is the thermal conductivity of the wire
#   - Q is the Joule heating
#
# The BCs are as follows:
#
#   curl(n) x curl(A) = 0,  where n is the normal vector
#   q * n = h (T - T_infty), where q is the heat flux,
#                            h is the convective heat transfer coefficient,
#                            and T_infty is the far-field temperature.

[Mesh]
  # Mesh of the copper wire
  [fmg]
    type = FileMeshGenerator
    file = copper_wire.msh
  []
[]

[Variables]
  # The real and complex components of the magnetic vector
  # potential in the frequency domain
  [A_real]
    family = NEDELEC_ONE
    order = FIRST
  []
  [A_imag]
    family = NEDELEC_ONE
    order = FIRST
  []

  # The temperature of the air in the copper wire
  [T]
    initial_condition = 293.0 #in K
  []
[]

[Kernels]
  ### Physics to determine the magnetic vector potential propagation ###
  # The propagation of the real component
  [curl_curl_real]
    type = CurlCurlField
    variable = A_real
  []
  # Current induced by the electrical conductivity
  # of the copper wire
  [conduction_real]
    type = ADConductionCurrent
    variable = A_real
    field_imag =  A_imag
    field_real =  A_real
    conductivity_real = electrical_conductivity
    conductivity_imag = 0.0
    ang_freq_real = omega_real
    ang_freq_imag = 0.0
    permeability_real = mu_real
    permeability_imag = 0.0
    component = real
  []
  # Current supplied to the wire
  [source_real]
    type = VectorBodyForce
    variable = A_real
    function = mu_curr_real
  []

  # The propagation of the complex component
  [curl_curl_imag]
    type = CurlCurlField
    variable = A_imag
  []
  # Current induced by the electrical conductivity
  # of the copper wire
  [conduction_imag]
    type = ADConductionCurrent
    variable = A_imag
    field_imag =  A_imag
    field_real =  A_real
    conductivity_real = electrical_conductivity
    conductivity_imag = 0.0
    ang_freq_real = omega_real
    ang_freq_imag = 0.0
    permeability_real = mu_real
    permeability_imag = 0.0
    component = imaginary
  []

  ### Physics to determine the heat transfer ###
  # Heat transfer in the copper wire
  [HeatTdot_in_copper]
    type = ADHeatConductionTimeDerivative
    variable = T
    specific_heat = specific_heat_copper
    density_name = density_copper
  []
  [HeatDiff_in_copper]
    type = ADHeatConduction
    variable = T
    thermal_conductivity = thermal_conductivity_copper
  []
  # Heating due the total current
  [HeatSrc]
    type = ADJouleHeatingSource
    variable = T
    heating_term = 'electric_field_heating'
  []
[]

[AuxVariables]
  # Decomposing the magnetic vector potential
  # for the electric field calculations
  [A_x_real]
    family = MONOMIAL
    order = FIRST
  []
  [A_y_real]
    family = MONOMIAL
    order = FIRST
  []

  [A_x_imag]
    family = MONOMIAL
    order = FIRST
  []
  [A_y_imag]
    family = MONOMIAL
    order = FIRST
  []

  # The electrical conductivity for the electric
  # field calculations
  [elec_cond]
    family = MONOMIAL
    order = FIRST
  []

  # The electric field profile determined from
  # the magnetic vector potential
  [E_real]
    family = NEDELEC_ONE
    order = FIRST
  []
  [E_imag]
    family = NEDELEC_ONE
    order = FIRST
  []
[]

[AuxKernels]
  # Decomposing the magnetic vector potential
  # for the electric field calculations
  [A_x_real]
    type = VectorVariableComponentAux
    variable = A_x_real
    vector_variable = A_real
    component = X
  []
  [A_y_real]
    type = VectorVariableComponentAux
    variable = A_y_real
    vector_variable = A_real
    component = Y
  []

  [A_x_imag]
    type = VectorVariableComponentAux
    variable = A_x_imag
    vector_variable = A_imag
    component = X
  []
  [A_y_imag]
    type = VectorVariableComponentAux
    variable = A_y_imag
    vector_variable = A_imag
    component = Y
  []

  # The electrical conductivity for the electric
  # field calculations
  [cond]
    type = ADMaterialRealAux
    property = electrical_conductivity
    variable = elec_cond
    execute_on = 'INITIAL LINEAR TIMESTEP_END'
  []

  # The magnitude of electric field profile determined
  # from the magnetic vector potential using:
  # abs(E) = abs(-j*omega*A) + abs(supplied current / elec_cond)
  # NOTE: The reason for calculating the magnitude of the electric
  #       field is the heating term is defined as:
  #       Q = 1/2 abs(E)^2 for frequency domain field formulations
  [E_real]
    type = ParsedVectorAux
    coupled_variables = 'A_x_imag A_y_imag elec_cond'
    expression_x = 'abs(2*3.14*60*A_x_imag) + abs(60e6/elec_cond)'
    expression_y = 'abs(2*3.14*60*A_y_imag)'
    variable = E_real
  []
  [E_imag]
    type = ParsedVectorAux
    coupled_variables = 'A_x_real A_y_real'
    expression_x = 'abs(-2*3.14*60*A_x_real)'
    expression_y = 'abs(-2*3.14*60*A_y_real)'
    variable = E_imag
  []
[]

[Functions]
  # The supplied current density to the wire
  # where only the real x-component is considered
  [curr_real_x]
    type = ParsedFunction
    expression = '60e6' # Units in A/m^2, equivalent to 1178 A in a 5mm diameter wire
  []

  # Permeability of free space
  [mu_real_func]
    type = ParsedFunction
    expression = '4*pi*1e-7' # Units in N/A^2
  []
  # The angular drive frequency of the system
  [omega_real_func]
    type = ParsedFunction
    expression = '2*pi*60' # Units in rad/s
  []

  # The angular frequency time permeability of free space
  [omegaMu]
    type = ParsedFunction
    symbol_names = 'omega mu'
    symbol_values = 'omega_real_func mu_real_func'
    expression = 'omega*mu'
  []

  # The supplied current density time permeability of free space
  [mu_curr_real]
    type = ParsedVectorFunction
    symbol_names = 'current_mag mu'
    symbol_values = 'curr_real_x mu_real_func'
    expression_x = 'mu * current_mag'
  []
[]

[BCs]
  ### Temperature boundary conditions ###
  # Convective heat flux BC with copper wire
  # exposed to air
  [surface]
    type = ADConvectiveHeatFluxBC
    variable = T
    boundary = walls
    T_infinity = 293
    heat_transfer_coefficient = 10
  []

  ### Magnetic vector potential boundary conditions ###
  # No defined boundary conditions represents
  # zero curl conditions at the boundaries, such that:
  # A x n = 0
[]

[Materials]
  [k]
    type = ADGenericConstantMaterial
    prop_names = 'thermal_conductivity_copper'
    prop_values = '397.48' #in W/(m K)
  []
  [cp]
    type = ADGenericConstantMaterial
    prop_names = 'specific_heat_copper'
    prop_values = '385.0' #in J/(kg K)
  []
  [rho]
    type = ADGenericConstantMaterial
    prop_names = 'density_copper'
    prop_values = '8920.0' #in kg/(m^3)
  []

  # Electrical conductivity (copper is default material)
  [sigma]
    type = ADElectricalConductivity
    temperature = T
    block = copper
  []
  # Material that supplies the correct Joule heating formulation
  [ElectromagneticMaterial]
    type = ElectromagneticHeatingMaterial
    electric_field = E_real
    complex_electric_field = E_imag
    electric_field_heating_name = electric_field_heating
    electrical_conductivity = electrical_conductivity
    formulation = FREQUENCY
    solver = ELECTROMAGNETIC
    block = copper
  []

  # Coefficient for wave propagation
  [mu_real]
    type = ADGenericFunctionMaterial
    prop_names = mu_real
    prop_values = mu_real_func
  []
  [omega_real]
    type = ADGenericFunctionMaterial
    prop_names = omega_real
    prop_values = omega_real_func
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = NEWTON
  line_search = NONE
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  dt = 1.0
  end_time = 10
  automatic_scaling = true
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
