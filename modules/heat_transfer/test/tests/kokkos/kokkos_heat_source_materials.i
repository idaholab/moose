# Exercises the Kokkos ports of FunctionPathEllipsoidHeatSource, ElectricalConductivity,
# ThermalCompliance, and JouleHeatingHeatGeneratedAux by sampling their material properties in a
# transient heat conduction problem with a nonuniform temperature field.

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -5
  xmax = 5
  ymin = -5
  ymax = 5
  nx = 10
  ny = 10
[]

[Variables]
  [temp]
    initial_condition = 300
  []
[]

[AuxVariables]
  [joule_heating]
    order = CONSTANT
    family = MONOMIAL
  []
  [compliance]
    order = CONSTANT
    family = MONOMIAL
  []
  [conductivity]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Functions]
  [path_x]
    type = KokkosParsedFunction
    expression = 2*cos(2*pi*t)
  []
  [path_y]
    type = KokkosParsedFunction
    expression = 2*sin(2*pi*t)
  []
[]

[Kernels]
  [time]
    type = KokkosHeatConductionTimeDerivative
    variable = temp
  []
  [heat_conduct]
    type = KokkosHeatConduction
    variable = temp
  []
[]

[AuxKernels]
  [joule_heating]
    type = KokkosJouleHeatingHeatGeneratedAux
    variable = joule_heating
    heating_term = volumetric_heat
  []
  [compliance]
    type = KokkosMaterialRealAux
    variable = compliance
    property = thermal_compliance
  []
  [conductivity]
    type = KokkosMaterialRealAux
    variable = conductivity
    property = electrical_conductivity
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
    type = KokkosDirichletBC
    variable = temp
    boundary = right
    value = 300
  []
[]

[Materials]
  [heat]
    type = KokkosHeatConductionMaterial
    specific_heat = 603
    thermal_conductivity = 10
  []
  [density]
    type = KokkosGenericConstantMaterial
    prop_names = density
    prop_values = 1e-3
  []
  [volumetric_heat]
    type = KokkosFunctionPathEllipsoidHeatSource
    rx = 1
    ry = 1
    rz = 1
    power = 1000
    efficiency = 0.5
    factor = 2
    function_x = path_x
    function_y = path_y
  []
  [electrical_conductivity]
    type = KokkosElectricalConductivity
    temperature = temp
  []
  [compliance]
    type = KokkosThermalCompliance
    temperature = temp
    thermal_conductivity = thermal_conductivity
  []
[]

[Executioner]
  type = Transient
  num_steps = 4
  dt = 0.1
  nl_abs_tol = 1e-10
[]

[Outputs]
  exodus = true
[]
