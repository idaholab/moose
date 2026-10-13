# Exercises the Kokkos ports of objects with temperature-dependent properties and their
# derivatives: KokkosHeatConductionMaterial with temperature functions,
# KokkosHeatCapacityConductionTimeDerivative with a coupled derivative, KokkosHeatConductionBC,
# KokkosThermalSensitivity, and KokkosThermalConductivity.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 4
[]

[Variables]
  [temp]
    initial_condition = 300
  []
  [rho]
    initial_condition = 0.5
  []
[]

[AuxVariables]
  [sensitivity]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Functions]
  [k_of_T]
    type = KokkosLinearTimeFunction
    a = 5
    b = 0.01
  []
  [cp_of_T]
    type = KokkosLinearTimeFunction
    a = 100
    b = 0.1
  []
[]

[Kernels]
  [time]
    type = KokkosHeatCapacityConductionTimeDerivative
    variable = temp
    coupled_variables = rho
  []
  [heat_conduct]
    type = KokkosHeatConduction
    variable = temp
    d_thermal_conductivity_dT = thermal_conductivity_dT
  []
  [rho_time]
    type = KokkosTimeDerivative
    variable = rho
  []
  [rho_diff]
    type = KokkosDiffusion
    variable = rho
  []
[]

[AuxKernels]
  [sensitivity]
    type = KokkosMaterialRealAux
    variable = sensitivity
    property = thermal_sensitivity
  []
[]

[BCs]
  [hot]
    type = KokkosDirichletBC
    variable = temp
    boundary = left
    value = 600
  []
  [cold]
    type = KokkosConvectiveHeatFluxBC
    variable = temp
    boundary = right
    T_infinity = 300
    heat_transfer_coefficient = 10
  []
  [conduction]
    # adds -k grad(T) . n, canceling the natural flux on the top boundary
    type = KokkosHeatConductionBC
    variable = temp
    boundary = top
  []
  [rho_left]
    type = KokkosDirichletBC
    variable = rho
    boundary = left
    value = 1
  []
[]

[Materials]
  [heat]
    type = KokkosHeatConductionMaterial
    temperature = temp
    thermal_conductivity_temperature_function = k_of_T
    specific_heat_temperature_function = cp_of_T
  []
  [heat_capacity]
    # C = rho * cp with the derivative with respect to rho for the off-diagonal Jacobian
    type = KokkosParsedMaterial
    property_name = heat_capacity
    coupled_variables = rho
    material_property_names = specific_heat
    expression = 'rho * specific_heat'
  []
  [dheat_capacity_drho]
    type = KokkosParsedMaterial
    property_name = 'dheat_capacity/drho'
    material_property_names = specific_heat
    expression = specific_heat
  []
  [dheat_capacity_dtemp]
    # rho * dcp/dT, since cp depends on the temperature through the specific heat function
    type = KokkosParsedMaterial
    property_name = 'dheat_capacity/dtemp'
    coupled_variables = rho
    material_property_names = specific_heat_dT
    expression = 'rho * specific_heat_dT'
  []
  [dthermal_conductivity_drho]
    type = KokkosGenericConstantMaterial
    prop_names = 'dthermal_conductivity/drho'
    prop_values = 2
  []
  [sensitivity]
    type = KokkosThermalSensitivity
    temperature = temp
    design_density = rho
    thermal_conductivity = thermal_conductivity
  []
[]

[Postprocessors]
  [T_hot]
    type = KokkosSideAverageValue
    variable = temp
    boundary = left
  []
  [flux]
    type = KokkosSideAverageValue
    variable = temp
    boundary = right
  []
  [k_eff]
    type = KokkosThermalConductivity
    variable = temp
    boundary = right
    dx = 1
    flux = flux
    T_hot = T_hot
    length_scale = 1
    k0 = 5
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 3
  dt = 0.5
  nl_abs_tol = 1e-10
[]

[Outputs]
  exodus = true
[]
