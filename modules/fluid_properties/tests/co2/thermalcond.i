# Test thermophysical property calculations in CO2FluidProperties
#
# Comparison with thermal conductivity from
# Scalabrin et al., A Reference Multiparameter Thermal Conductivity Equation
# for Carbon Dioxide with an Optimized Functional Form,
# J. Phys. Chem. Ref. Data 35 (2006)
#
#  --------------------------------------------------------------
#  Pressure (Mpa)             |   1       |    1      |   1
#  Temperature (K)            |  250      |  300      |  450
#  --------------------------------------------------------------
#  Expected values
#  --------------------------------------------------------------
#  Thermal cond. (mW/m/K)     |  13.45    |  17.248   |  29.377
#  --------------------------------------------------------------
#  Calculated values
#  --------------------------------------------------------------
#  Thermal cond. (mW/m/K)     |  13.45    |  17.248   |  29.377
#  --------------------------------------------------------------

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
  [./k]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[Functions]
  [./tic]
    type = ParsedFunction
    value = if(x<1,250,if(x<2,300,450))
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
  [./k]
    type = MaterialRealAux
    variable = k
    property = k
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
  [./k0]
    type = ElementalVariableValue
    elementid = 0
    variable = k
  [../]
  [./k1]
    type = ElementalVariableValue
    elementid = 1
    variable = k
  [../]
  [./k2]
    type = ElementalVariableValue
    elementid = 2
    variable = k
  [../]
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
