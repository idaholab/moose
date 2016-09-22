# Test melting pressure calculation in CO2FluidProperties
#
# Expected output taken from experimental data from
# Michels et al, The melting line of carbon dioxide up to
# 2800 atmospheres, Physica 9 (1942). Note that results in
# this reference are given in atm, but have been converted to MPa here
#
# Three input temperatures are used, and the expected results are
# -----------------------------------------------
# Temperature (K) |  217.03  |  235.29  |  266.04
# Pressure (MPa)  |  2.57    |  95.86   |  286.77
# -----------------------------------------------
#
# Results calculated by CO2FluidProperties are
# -----------------------------------------------
# Temperature (K) |  217.03  |  235.29  |  266.04
# Pressure (MPa)  |  2.57    |  95.89   |  287.25
# -----------------------------------------------
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
  [./pmelt]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[Functions]
  [./tic]
    type = ParsedFunction
    value = if(x<1,217.03,if(x<2,235.29,266.04))
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
  [./pmelt]
    type = MaterialRealAux
    variable = pmelt
    property = melting_pressure
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
    melting = true
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
  [./pmelt0]
    type = ElementalVariableValue
    elementid = 0
    variable = pmelt
  [../]
  [./pmelt1]
    type = ElementalVariableValue
    elementid = 1
    variable = pmelt
  [../]
  [./pmelt2]
    type = ElementalVariableValue
    elementid = 2
    variable = pmelt
  [../]
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
