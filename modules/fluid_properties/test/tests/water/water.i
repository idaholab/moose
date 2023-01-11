# Example of using Water97FluidProperties module in Region 1 by recovering the values
# in Table 5 of Revised Release on the IAPWS Industrial Formulation 1997 for the
# Thermodynamic Properties of Water and Steam

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  xmax = 3
  # This test uses ElementalVariableValue postprocessors on specific
  # elements, so element numbering needs to stay unchanged
  allow_renumbering = false
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
  [./v]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./e]
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
  [./mu]
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
    expression = 'if(x<2, 300, 500)'
  [../]
  [./pic]
    type = ParsedFunction
    expression = 'if(x<1,3e6, if(x<2, 80e6, 3e6))'
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
  [./v]
    type = ParsedAux
    coupled_variables = rho
    expression = 1/rho
    variable = v
  [../]
  [./e]
    type = MaterialRealAux
    variable = e
    property = e
  [../]
  [./h]
    type = MaterialRealAux
    variable = h
    property = h
  [../]
  [./s]
    type = MaterialRealAux
    variable = s
    property = s
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
  [./mu]
    type = MaterialRealAux
    variable = mu
    property = viscosity
  [../]
  [./k]
    type = MaterialRealAux
    variable = k
    property = k
  [../]
[]

[FluidProperties]
  [./water]
    type = Water97FluidProperties
  [../]
[]

[Materials]
  [./fp_mat]
    type = FluidPropertiesMaterialPT
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
  [./v0]
    type = ElementalVariableValue
    variable = v
    elementid = 0
  [../]
  [./v1]
    type = ElementalVariableValue
    variable = v
    elementid = 1
  [../]
  [./v2]
    type = ElementalVariableValue
    variable = v
    elementid = 2
  [../]
  [./e0]
    type = ElementalVariableValue
    variable = e
    elementid = 0
  [../]
  [./e1]
    type = ElementalVariableValue
    variable = e
    elementid = 1
  [../]
  [./e2]
    type = ElementalVariableValue
    variable = e
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
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  csv = true
[]
