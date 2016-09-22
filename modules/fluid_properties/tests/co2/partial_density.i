# Test partial density calculation in CO2FluidProperties
#
# Expected output taken from
# Hnedkovsky et al., Volumes of aqueous solutions of CH4, CO2, H2S,
# and NH3 at temperatures from 298.15 K to 705 K and pressures to 35 MPa,
# J. Chem. Thermo. 28, 1996
#
# Three input temperatures are used, and the expected results are
# -------------------------------------------------
# Temperature (K)   |  373.15  |  473.35  |  573.15
# Density (kg/m^3)  |  1182.8  |  880.0   |  593.8
# -------------------------------------------------
#
# Results calculated by CO2FluidProperties are
# -------------------------------------------------
# Temperature (K)   |  373.15  |  473.35  |  573.15
# Density (kg/m^3)  |  1217.1  |  892.6   |  596.4
# -------------------------------------------------
#
# These results are within the expected accuracy

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
  [./rhop]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[Functions]
  [./tic]
    type = ParsedFunction
    value = if(x<1,373.15,if(x<2,473.35,573.15))
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
  [./rhop]
    type = MaterialRealAux
    variable = rhop
    property = partial_density
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
  [./rhop0]
    type = ElementalVariableValue
    elementid = 0
    variable = rhop
  [../]
  [./rhop1]
    type = ElementalVariableValue
    elementid = 1
    variable = rhop
  [../]
  [./rhop2]
    type = ElementalVariableValue
    elementid = 2
    variable = rhop
  [../]
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
