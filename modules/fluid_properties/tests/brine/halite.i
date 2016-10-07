# Test BrineFluidProperties calculations of halite density, cp and enthalpy
#
# Experimental data for density from Brown, "The NaCl pressure standard",
# J. Appl. Phys., 86 (1999)
#
#  --------------------------------------------------------------
#  Temperature (K)                 |   300    |   500   |   700
#  Pressure (Mpa)                  |    30    |    60   |    80
#  --------------------------------------------------------------
#  Expected values
#  --------------------------------------------------------------
#  Halite density (kg/m^3)         |  2167.8  |  2116.0 |  2056.8
#  --------------------------------------------------------------
#  Calculated values
#  --------------------------------------------------------------
#  Halite density (kg/m^3)         |  2166.3  |  2115.4 |  2056.4
#  --------------------------------------------------------------
#
# Values for cp and enthalpy are difficult to compare against. Instead, the
# values provided by the BrineFluidProperties UserObject were compared against
# simple correlations, eg. from NIST sodium chloride data. For the conditions above,
# the expected values of cp are approximately:
#  --------------------------------------------------------------
#  Expected values
#  --------------------------------------------------------------
#  Halite specific heat (kJ/kg/K)  |   0.865  |  0.922  |  0.979
#  --------------------------------------------------------------
#  Calculated values
#  --------------------------------------------------------------
#  Halite specific heat (kJ/kg/K)  |   0.875  |  0.904  |  0.949
#  --------------------------------------------------------------
#
# These values are within expected accuracy

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
  [../]
  [./temperature]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./halite_density]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./halite_cp]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./halite_enthalpy]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxKernels]
  [./density]
    type = MaterialRealAux
     variable = halite_density
     property = halite_density
  [../]
  [./cp]
    type = MaterialRealAux
     variable = halite_cp
     property = halite_cp
  [../]
  [./enthalpy]
    type = MaterialRealAux
     variable = halite_enthalpy
     property = halite_enthalpy
  [../]
[]

[Functions]
  [./tic]
    type = ParsedFunction
    value = 'if(x<1, 300, if(x<2, 500, 700))'
  [../]
  [./pic]
    type = ParsedFunction
    value = 'if(x<1, 30e6, if(x<2, 60e6, 80e6))'
  [../]
[]

[ICs]
  [./t_ic]
    type = FunctionIC
    function = tic
    variable = temperature
  [../]
  [./p_ic]
    type = FunctionIC
    function = pic
    variable = pressure
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
    xnacl = 0
    fp = brine
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
  [./density0]
    type = ElementalVariableValue
    variable = halite_density
    elementid = 0
  [../]
  [./density1]
    type = ElementalVariableValue
    variable = halite_density
    elementid = 1
  [../]
  [./density2]
    type = ElementalVariableValue
    variable = halite_density
    elementid = 2
  [../]
  [./cp0]
    type = ElementalVariableValue
    variable = halite_cp
    elementid = 0
  [../]
  [./cp1]
    type = ElementalVariableValue
    variable = halite_cp
    elementid = 1
  [../]
  [./cp2]
    type = ElementalVariableValue
    variable = halite_cp
    elementid = 2
  [../]
  [./enthlapy0]
    type = ElementalVariableValue
    variable = halite_enthalpy
    elementid = 0
  [../]
  [./enthlapy1]
    type = ElementalVariableValue
    variable = halite_enthalpy
    elementid = 1
  [../]
  [./enthlapy2]
    type = ElementalVariableValue
    variable = halite_enthalpy
    elementid = 2
  [../]
[]

[Outputs]
  csv = true
  execute_on = 'timestep_end'
[]
