# Test the Water97FluidProperties module in region 4 (saturation line) by recovering
# the values in Tables 35 and 36 of Revised Release on the IAPWS Industrial Formulation
# 1997 for the Thermodynamic Properties of Water and Steam

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
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./temperature]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./psat]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./Tsat]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[Functions]
  [./tic]
    type = ParsedFunction
    value = 'if(x<1, 300, if(x<2, 500, 600))'
  [../]
  [./pic]
    type = ParsedFunction
    value = 'if(x<1,0.1e6, if(x<2, 1e6, 10e6))'
  [../]
[]

[ICs]
  [./p_ic]
    type = FunctionIC
    function = pic
    variable = pressure
  [../]
  [./t_ic]
    type = FunctionIC
    function = tic
    variable = temperature
  [../]
[]

[AuxKernels]
  [./psat]
    type = MaterialRealAux
    variable = psat
    property = psat
  [../]
  [./Tsat]
    type = MaterialRealAux
    variable = Tsat
    property = Tsat
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
    temperature = temperature
    fp = water
    region4 = true
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = dummy
  [../]
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
  [./Tsat0]
    type = ElementalVariableValue
    variable = Tsat
    elementid = 0
  [../]
  [./Tsat1]
    type = ElementalVariableValue
    variable = Tsat
    elementid = 1
  [../]
  [./Tsat2]
    type = ElementalVariableValue
    variable = Tsat
    elementid = 2
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  csv = true
[]
