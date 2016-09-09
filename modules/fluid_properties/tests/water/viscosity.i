# Test the Water97FluidProperties viscosity by recovering the values
# in Table 4 of Release on the IAPWS Formulation 2008 for the Viscosity of
# Ordinary Water Substance.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 11
  xmax = 11
[]

[Variables]
  [./dummy]
  [../]
[]

[AuxVariables]
  [./density]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./temperature]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./mu]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./pressure]
    initial_condition = 1e5
  [../]
[]

[Functions]
  [./tic]
    type = ParsedFunction
    value = 'if(x<2, 298.15, if(x<3, 373.15, if(x<5, 433.15, if(x<8, 873.15, 1173.15))))'
  [../]
  [./rhoic]
    type = ParsedFunction
    value = 'if(x<1,998, if(x<2, 1200, 1000))'
  [../]
[]

[ICs]
  [./rho_ic]
    type = FunctionIC
    function = rhoic
    variable = density
  [../]
  [./t_ic]
    type = FunctionIC
    function = tic
    variable = temperature
  [../]
[]

[AuxKernels]
  [./mu]
    type = MaterialRealAux
    variable = mu
    property = viscosity
  [../]
[]

[Modules]
  [./FluidProperties]
    [./water]
      type = Water97FluidProperties
    [../]
  [../]
[]

[Materials]
  [./fp_mat]
    type = WaterFluidPropertiesTestMaterial
    pressure = pressure
    density = density
    temperature = temperature
    viscosity = true
    fp = water
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = dummy
  [../]
[]

[Postprocessors]
  [./mu0]
    type = ElementalVariableValue
    variable = mu
    elementid = 0
  [../]
  [./mu1]
    type = ElementalVariableValue
    variable = mu
    elementid = 1
  [../]
  [./mu2]
    type = ElementalVariableValue
    variable = mu
    elementid = 2
  [../]
  [./mu3]
    type = ElementalVariableValue
    variable = mu
    elementid = 3
  [../]
  [./mu4]
    type = ElementalVariableValue
    variable = mu
    elementid = 4
  [../]
  [./mu5]
    type = ElementalVariableValue
    variable = mu
    elementid = 5
  [../]
  [./mu6]
    type = ElementalVariableValue
    variable = mu
    elementid = 6
  [../]
  [./mu7]
    type = ElementalVariableValue
    variable = mu
    elementid = 7
  [../]
  [./mu8]
    type = ElementalVariableValue
    variable = mu
    elementid = 8
  [../]
  [./mu9]
    type = ElementalVariableValue
    variable = mu
    elementid = 9
  [../]
  [./mu10]
    type = ElementalVariableValue
    variable = mu
    elementid = 10
  [../]
  [./mu11]
    type = ElementalVariableValue
    variable = mu
    elementid = 11
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  csv = true
[]
