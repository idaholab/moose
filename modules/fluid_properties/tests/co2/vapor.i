# Test vapor pressure, saturated vapor density and saturated liquid density
# calculation in CO2FluidProperties
#
# Expected output taken from experimental data from
# Duschek et al., Measurement and correlation of the (pressure, density, temperature)
# relation of cabon dioxide II. Saturated-liquid and saturated-vapor densities and
# the vapor pressure along the entire coexstance curve, J. Chem. Thermo. 22 (1990)
#
# Three input temperatures are used, and the expected results are
# -----------------------------------------------------------
# Temperature (K)          |  217.0    |  245.0    |  303.8
# Pressure (MPa)           |  0.52747  |  1.51887  |  7.32029
# Vapor density (kg.m^3)  |  14.0017  |  39.5048  |  382.30
# Liquid density (kg.m^3)  |  1177.03  |  1067.89  |  554.14
# -----------------------------------------------------------
#
# Results calculated by CO2FluidProperties are
# -----------------------------------------------------------
# Temperature (K)          |  217.0    |  245.0    |  303.8
# Pressure (MPa)           |  0.52721  |  1.51854  |  7.32174
# Vapor density (kg.m^3)  |  13.9958  |  39.4981  |  383.093
# Liquid density (kg.m^3)  |  1177.06  |  1067.91  |  553.20
# -----------------------------------------------------------
#
# These are within the expected uncertainty

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  xmax = 3
[]

[Variables]
  [./dummy]
  [../]
[]

[AuxVariables]
  [./pressure]
    initial_condition = 1e6
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./temperature]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./pvap]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./rhovap]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./rhosat]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[Functions]
  [./tic]
    type = ParsedFunction
    value = if(x<1,217,if(x<2,245,303.8))
  [../]
[]

[ICs]
  [./t_ic]
    type = FunctionIC
    function = tic
    variable = temperature
  [../]
[]

[AuxKernels]
  [./pvap]
    type = MaterialRealAux
     variable = pvap
     property = vapor_pressure
  [../]
  [./rhovap]
    type = MaterialRealAux
    variable = rhovap
    property = saturated_vapor_density
  [../]
  [./rhosat]
    type = MaterialRealAux
    variable = rhosat
    property = saturated_liquid_density
  [../]
[]

[Modules]
  [./FluidProperties]
    [./co2]
      type = CO2FluidProperties
    [../]
  []
[]

[Materials]
  [./fp_mat]
    type = CO2FluidPropertiesTestMaterial
    pressure = pressure
    temperature = temperature
    fp = co2
    vapor = true
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = dummy
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Postprocessors]
  [./pvap0]
    type = ElementalVariableValue
    elementid = 0
    variable = pvap
  [../]
  [./pvap1]
    type = ElementalVariableValue
    elementid = 1
    variable = pvap
  [../]
  [./pvap2]
    type = ElementalVariableValue
    elementid = 2
    variable = pvap
  [../]
  [./rhovap0]
    type = ElementalVariableValue
    elementid = 0
    variable = rhovap
  [../]
  [./rhovap1]
    type = ElementalVariableValue
    elementid = 1
    variable = rhovap
  [../]
  [./rhovap2]
    type = ElementalVariableValue
    elementid = 2
    variable = rhovap
  [../]
  [./rhosat0]
    type = ElementalVariableValue
    elementid = 0
    variable = rhosat
  [../]
  [./rhosat1]
    type = ElementalVariableValue
    elementid = 1
    variable = rhosat
  [../]
  [./rhosat2]
    type = ElementalVariableValue
    elementid = 2
    variable = rhosat
  [../]
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
