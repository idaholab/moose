# Test thermophysical property calculations in CO2FluidProperties
#
# Comparison with values from Span and Wagner, "A New Equation of State for
# Carbon Dioxide Covering the Fluid Region from the Triple-Point Temperature
# to 1100K at Pressures up to 800 MPa", J. Phys. Chem. Ref. Data, 25 (1996)
#
# Viscosity values from Fenghour et al., "The viscosity of carbon dioxide",
# J. Phys. Chem. Ref. Data, 27, 31-44 (1998)
#
#
#  --------------------------------------------------------------
#  Pressure (Mpa)             |   1       |    1      |   1
#  Temperature (K)            |  280      |  360      |  500
#  --------------------------------------------------------------
#  Expected values
#  --------------------------------------------------------------
#  Density (kg/m^3)           |  20.199   |  15.105   |  10.664
#  Internal energy (kJ/kg/K)  |  -75.892  |  -18.406  |  91.829
#  Enthalpy (kJ/kg)           |  -26.385  |  47.797   |  185.60
#  Entropy (kJ/kg/K)          |  -0.51326 |  -0.28033 |  0.04225
#  cv (kJ/kg/K)               |  0.67092  |  0.72664  |  0.82823
#  cp (kJ/kg/K)               |  0.92518  |  0.94206  |  1.0273
#  Speed of sound (m/s)       |  252.33   |  289.00   |  339.81
#  Viscosity (1e-6Pa.s)       |  14.15    |  17.94    |  24.06
#  --------------------------------------------------------------
#  Calculated values
#  --------------------------------------------------------------
#  Density (kg/m^3)           |  20.199   |  15.105   |  10.664
#  Internal energy (kJ/kg/K)  |  -75.892  |  -18.406  |  91.829
#  Enthalpy (kJ/kg)           |  -26.385  |  47.797   |  185.60
#  Entropy (kJ/kg/K)          |  -0.51326 |  -0.28033 |  0.04225
#  cv (kJ/kg/K)               |  0.67092  |  0.72664  |  0.82823
#  cp (kJ/kg/K)               |  0.92518  |  0.94206  |  1.0273
#  Speed of sound (m/s)       |  252.33   |  289.00   |  339.81
#  Viscosity (1e-6 Pa.s)      |  14.15    |  17.94    |  24.06
#  --------------------------------------------------------------

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  xmax = 3
  # This test uses ElementalVariableValue postprocessors on specific
  # elements, so element numbering needs to stay unchanged
  allow_renumbering = false
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
  [./rho]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./mu]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./e]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./h]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./s]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./cv]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./cp]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./c]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[Functions]
  [./tic]
    type = ParsedFunction
    expression = if(x<1,280,if(x<2,360,500))
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
  [./rho]
    type = MaterialRealAux
    variable = rho
    property = density
  [../]
  [./my]
    type = MaterialRealAux
    variable = mu
    property = viscosity
  [../]
  [./internal_energy]
    type = MaterialRealAux
    variable = e
    property = e
  [../]
  [./enthalpy]
    type = MaterialRealAux
    variable = h
    property = h
  [../]
  [./entropy]
    type = MaterialRealAux
    variable = s
    property = s
  [../]
  [./cv]
    type = MaterialRealAux
    variable = cv
    property = cv
  [../]
  [./cp]
    type = MaterialRealAux
    variable = cp
    property = cp
  [../]
  [./c]
    type = MaterialRealAux
    variable = c
    property = c
  [../]
[]

[FluidProperties]
  [./co2]
    type = CO2FluidProperties
  [../]
[]

[Materials]
  [./fp_mat]
    type = FluidPropertiesMaterialPT
    pressure = pressure
    temperature = temperature
    fp = co2
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
  [./rho0]
    type = ElementalVariableValue
    elementid = 0
    variable = rho
  [../]
  [./rho1]
    type = ElementalVariableValue
    elementid = 1
    variable = rho
  [../]
  [./rho2]
    type = ElementalVariableValue
    elementid = 2
    variable = rho
  [../]
  [./mu0]
    type = ElementalVariableValue
    elementid = 0
    variable = mu
  [../]
  [./mu1]
    type = ElementalVariableValue
    elementid = 1
    variable = mu
  [../]
  [./mu2]
    type = ElementalVariableValue
    elementid = 2
    variable = mu
  [../]
  [./e0]
    type = ElementalVariableValue
    elementid = 0
    variable = e
  [../]
  [./e1]
    type = ElementalVariableValue
    elementid = 1
    variable = e
  [../]
  [./e2]
    type = ElementalVariableValue
    elementid = 2
    variable = e
  [../]
  [./h0]
    type = ElementalVariableValue
    elementid = 0
    variable = h
  [../]
  [./h1]
    type = ElementalVariableValue
    elementid = 1
    variable = h
  [../]
  [./h2]
    type = ElementalVariableValue
    elementid = 2
    variable = h
  [../]
  [./s0]
    type = ElementalVariableValue
    elementid = 0
    variable = s
  [../]
  [./s1]
    type = ElementalVariableValue
    elementid = 1
    variable = s
  [../]
  [./s2]
    type = ElementalVariableValue
    elementid = 2
    variable = s
  [../]
  [./cv0]
    type = ElementalVariableValue
    elementid = 0
    variable = cv
  [../]
  [./cv1]
    type = ElementalVariableValue
    elementid = 1
    variable = cv
  [../]
  [./cv2]
    type = ElementalVariableValue
    elementid = 2
    variable = cv
  [../]
  [./cp0]
    type = ElementalVariableValue
    elementid = 0
    variable = cp
  [../]
  [./cp1]
    type = ElementalVariableValue
    elementid = 1
    variable = cp
  [../]
  [./cp2]
    type = ElementalVariableValue
    elementid = 2
    variable = cp
  [../]
  [./c0]
    type = ElementalVariableValue
    elementid = 0
    variable = c
  [../]
  [./c1]
    type = ElementalVariableValue
    elementid = 1
    variable = c
  [../]
  [./c2]
    type = ElementalVariableValue
    elementid = 2
    variable = c
  [../]
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
