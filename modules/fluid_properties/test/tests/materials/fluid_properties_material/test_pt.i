[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  elem_type = QUAD4
[]

[Functions]
  [fn_1]
    type = ParsedFunction
    expression = '2e5 * (1 + x)'
  []
  [fn_2]
    type = ParsedFunction
    expression = '300 * (1 + x*x+y*y)'
  []
[]

[AuxVariables]
  [pressure]
    [InitialCondition]
      type = FunctionIC
      function = fn_1
    []
  []
  [temperature]
    [InitialCondition]
      type = FunctionIC
      function = fn_2
    []
  []

  [rho]
    family = MONOMIAL
    order = CONSTANT
  []
  [mu]
    family = MONOMIAL
    order = CONSTANT
  []
  [cp]
    family = MONOMIAL
    order = CONSTANT
  []
  [cv]
    family = MONOMIAL
    order = CONSTANT
  []
  [k]
    family = MONOMIAL
    order = CONSTANT
  []
  [h]
    family = MONOMIAL
    order = CONSTANT
  []
  [e]
    family = MONOMIAL
    order = CONSTANT
  []
  [s]
    family = MONOMIAL
    order = CONSTANT
  []
  [c]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [rho]
    type = MaterialRealAux
     variable = rho
     property = density
  []
  [mu]
    type = MaterialRealAux
     variable = mu
     property = viscosity
  []
  [cp]
    type = MaterialRealAux
     variable = cp
     property = cp
  []
  [cv]
    type = MaterialRealAux
     variable = cv
     property = cv
  []
  [k]
    type = MaterialRealAux
     variable = k
     property = k
  []
  [h]
    type = MaterialRealAux
     variable = h
     property = h
  []
  [e]
    type = MaterialRealAux
     variable = e
     property = e
  []
  [s]
    type = MaterialRealAux
     variable = s
     property = s
  []
  [c]
    type = MaterialRealAux
     variable = c
     property = c
  []
[]

[FluidProperties]
  [ideal_gas]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 1.000536678700361
  []
[]

[Materials]
  [fp_mat]
    type = FluidPropertiesMaterialPT
    pressure = pressure
    temperature = temperature
    fp = ideal_gas
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Problem]
  solve = false
[]

[Outputs]
  exodus = true
[]
