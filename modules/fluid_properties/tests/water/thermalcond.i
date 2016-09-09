# Test the Water97FluidProperties thermal conductivity by recovering the values
# in Table D1 of Revised Release on the IAPS Formulation 1985 for the Thermal
# Conductivity of Ordinary Water Substance
#
#  ----------------------------------------------------------------
#  Temperature (C)       |   50     |  350     |  500     |  800
#  Pressure (Mpa)        |    1     |  20      |   50     |   95
#  Expected k (W/m/K)    |  0.641   |  0.4541  |  0.2055  |  0.2056
#  Calculated k (W/m/K)  |  0.6410  |  0.4541  |  0.2055  |  0.2056
#  ----------------------------------------------------------------

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
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./temperature]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./k]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[Functions]
  [./tic]
    type = ParsedFunction
    value = 'if(x<1, 50, if(x<2,350, if(x<3,500,800)))+273.15'
  [../]
  [./pic]
    type = ParsedFunction
    value = 'if(x<1,1, if(x<2, 20, if(x<3, 50, 95)))*1e6'
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
  [./k]
    type = MaterialRealAux
    variable = k
    property = k
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
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = dummy
  [../]
[]

[Postprocessors]
  [./k0]
    type = ElementalVariableValue
    variable = k
    elementid = 0
  [../]
  [./k1]
    type = ElementalVariableValue
    variable = k
    elementid = 1
  [../]
  [./k2]
    type = ElementalVariableValue
    variable = k
    elementid = 2
  [../]
  [./k3]
    type = ElementalVariableValue
    variable = k
    elementid = 3
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  csv = true
[]
