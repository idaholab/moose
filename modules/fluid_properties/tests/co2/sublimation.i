# Test sublimation pressure calculation in CO2FluidProperties
#
# Expected output P = 0.101325 MPa at T = 194.6857
# From Bedford et al., Recommended values of temperature for a
# selected set of secondary reference points, Metrologia 20 (1984)
#
# Result calculated by CO2FluidProperties is P = 0.101327 MPa

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
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
    initial_condition = 194.6857
  [../]
  [./psub]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxKernels]
  [./psub]
    type = MaterialRealAux
    variable = psub
    property = sublimation_pressure
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
    sublimation = true
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
  [./psub]
    type = ElementalVariableValue
    elementid = 0
    variable = psub
  [../]
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
