# Test BrineFluidProperties calculations of brine vapor pressure
#
# Experimental data from Liu and Lindsay, Thermodynamics of sodium chloride
# solutions at high temperatures, Journal of Solution Chemistry, 1, 45-69 (1972)
#
#  --------------------------------------------------------------
#  Temperature (C)               |   200   |   200   |   200
#  NaCl molality (mol/kg)        |  3.886  |   6.24  |  7.757
#  NaCl mass fraction (kg/kg)    |  0.185  |  0.267  |  0.312
#  --------------------------------------------------------------
#  Expected values
#  --------------------------------------------------------------
#  Vapor pressure (MPa)          |   1.34  |   1.21  |   1.13
#  --------------------------------------------------------------
#  Calculated values
#  --------------------------------------------------------------
#  Vapor pressure (MPa)          |   1.34  |   1.21  |   1.12
#  --------------------------------------------------------------
#
# These results are within expected accuracy

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 3
  xmax = 3
[]

[Variables]
  [./dummy]
  [../]
[]

[AuxVariables]
  [./pressure]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 1e6
  [../]
  [./temperature]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 473.15
  [../]
  [./xnacl]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./psat]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxKernels]
  [./psat]
    type = MaterialRealAux
     variable = psat
     property = vapor
  [../]
[]

[Functions]
  [./xic]
    type = ParsedFunction
    value = 'if(x<1, 0.185, if(x<2, 0.267, 0.317))'
  [../]
[]

[ICs]
  [./x_ic]
    type = FunctionIC
    function = xic
    variable = xnacl
  [../]
[]

[Modules]
  [./FluidProperties]
    [./brine]
      type = BrineFluidProperties
    [../]
  []
[]

[Materials]
  [./fp_mat]
    type = BrineFluidPropertiesTestMaterial
    pressure = pressure
    temperature = temperature
    xnacl = xnacl
    fp = brine
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
  [./psat0]
    type = ElementalVariableValue
    variable = psat
    elementid = 0
  [../]
  [./psat1]
    type = ElementalVariableValue
    variable = psat
    elementid = 1
  [../]
  [./psat2]
    type = ElementalVariableValue
    variable = psat
    elementid = 2
  [../]

[Outputs]
  csv = true
  execute_on = 'timestep_end'
[]
