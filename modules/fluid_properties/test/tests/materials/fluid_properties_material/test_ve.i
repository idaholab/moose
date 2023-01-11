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
    expression = '2000 + 100*x'
  []
  [fn_2]
    type = ParsedFunction
    expression = '0.02 * (x*x+y*y)'
  []
[]

[AuxVariables]
  [e]
    [InitialCondition]
      type = FunctionIC
      function = fn_1
    []
  []
  [v]
    [InitialCondition]
      type = FunctionIC
      function = fn_2
    []
  []

  [p]
    family = MONOMIAL
    order = CONSTANT
  []
  [T]
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
  [c]
    family = MONOMIAL
    order = CONSTANT
  []
  [mu]
    family = MONOMIAL
    order = CONSTANT
  []
  [k]
    family = MONOMIAL
    order = CONSTANT
  []
  [s]
    family = MONOMIAL
    order = CONSTANT
  []
  [g]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [p]
    type = MaterialRealAux
     variable = p
     property = pressure
  []
  [T]
    type = MaterialRealAux
     variable = T
     property = temperature
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
  [c]
    type = MaterialRealAux
     variable = c
     property = c
  []
  [mu]
    type = MaterialRealAux
     variable = mu
     property = mu
  []
  [k]
    type = MaterialRealAux
     variable = k
     property = k
  []
  [s]
    type = MaterialRealAux
     variable = s
     property = s
  []
  [g]
    type = MaterialRealAux
     variable = g
     property = g
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
    type = FluidPropertiesMaterialVE
    e = e
    v = v
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
