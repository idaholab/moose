# Test calculation of Henry's constant in CO2FluidProperties
#
# Comparison with values from "Guidelines on the Henry's constant and vapour
# liquid distribution constant for gases in H20 and D20 at high temperatures",
# IAPWS (2004).
#
#  --------------------------------------------------------------
#  Temperature (K)   |    300    |    400    |    500   |   600
#  --------------------------------------------------------------
#  Expected values
#  --------------------------------------------------------------
#  Kh (MPa)           |  173.63  |  579.84   |  520.79  |  259.53
#  --------------------------------------------------------------
#  Calculated values
#  --------------------------------------------------------------
#  Kh (MPa)           |  173.63  |  579.83   |  520.82  |  259.53
#  --------------------------------------------------------------
#
# Note: although there is a small discrepancy between the calculated results,
# when converted to ln(kh * 1e-9) as originally in the reference above, the
# calculated results are identical to the values in the reference

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  xmax = 4
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
  [./kh]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[Functions]
  [./tic]
    type = ParsedFunction
    value = if(x<1,300,if(x<2,400,if(x<3,500,600)))
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
  [./kh]
    type = MaterialRealAux
    variable = kh
    property = henry
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
  [./kh0]
    type = ElementalVariableValue
    elementid = 0
    variable = kh
  [../]
  [./kh1]
    type = ElementalVariableValue
    elementid = 1
    variable = kh
  [../]
  [./kh2]
    type = ElementalVariableValue
    elementid = 2
    variable = kh
  [../]
  [./kh3]
    type = ElementalVariableValue
    elementid = 3
    variable = kh
  [../]
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
