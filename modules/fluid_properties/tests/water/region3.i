# Test the Water97FluidProperties module in region 3 by recovering the values
# in Table 33 of Revised Release on the IAPWS Industrial Formulation 1997 for the
# Thermodynamic Properties of Water and Steam

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
  [./rho]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./u]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./h]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./s]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./cp]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./cv]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./c]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[Functions]
  [./tic]
    type = ParsedFunction
    value = 'if(x<2, 650, 750)'
  [../]
  [./pic]
    type = ParsedFunction
    value = 'if(x<1,25.588e6, if(x<2, 22.298e6, 78.32e6))'
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
  [./rho]
    type = MaterialRealAux
    variable = rho
    property = density
  [../]
  [./u]
    type = MaterialRealAux
    variable = u
    property = internal_energy
  [../]
  [./h]
    type = MaterialRealAux
    variable = h
    property = enthalpy
  [../]
  [./s]
    type = MaterialRealAux
    variable = s
    property = entropy
  [../]
  [./cp]
    type = MaterialRealAux
    variable = cp
    property = cp
  [../]
  [./cv]
    type = MaterialRealAux
    variable = cv
    property = cv
  [../]
  [./c]
    type = MaterialRealAux
    variable = c
    property = c
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
  [./density0]
    type = ElementalVariableValue
    variable = rho
    elementid = 0
  [../]
  [./density1]
    type = ElementalVariableValue
    variable = rho
    elementid = 1
  [../]
  [./density2]
    type = ElementalVariableValue
    variable = rho
    elementid = 2
  [../]
  [./u0]
    type = ElementalVariableValue
    variable = u
    elementid = 0
  [../]
  [./u1]
    type = ElementalVariableValue
    variable = u
    elementid = 1
  [../]
  [./u2]
    type = ElementalVariableValue
    variable = u
    elementid = 2
  [../]
  [./h0]
    type = ElementalVariableValue
    variable = h
    elementid = 0
  [../]
  [./h1]
    type = ElementalVariableValue
    variable = h
    elementid = 1
  [../]
  [./h2]
    type = ElementalVariableValue
    variable = h
    elementid = 2
  [../]
  [./s0]
    type = ElementalVariableValue
    variable = s
    elementid = 0
  [../]
  [./s1]
    type = ElementalVariableValue
    variable = s
    elementid = 1
  [../]
  [./s2]
    type = ElementalVariableValue
    variable = s
    elementid = 2
  [../]
  [./cp0]
    type = ElementalVariableValue
    variable = cp
    elementid = 0
  [../]
  [./cp1]
    type = ElementalVariableValue
    variable = cp
    elementid = 1
  [../]
  [./cp2]
    type = ElementalVariableValue
    variable = cp
    elementid = 2
  [../]
  [./cv0]
    type = ElementalVariableValue
    variable = cv
    elementid = 0
  [../]
  [./cv1]
    type = ElementalVariableValue
    variable = cv
    elementid = 1
  [../]
  [./cv2]
    type = ElementalVariableValue
    variable = cv
    elementid = 2
  [../]
  [./c0]
    type = ElementalVariableValue
    variable = c
    elementid = 0
  [../]
  [./c1]
    type = ElementalVariableValue
    variable = c
    elementid = 1
  [../]
  [./c2]
    type = ElementalVariableValue
    variable = c
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
