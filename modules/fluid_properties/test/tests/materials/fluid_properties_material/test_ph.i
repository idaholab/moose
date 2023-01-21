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
    expression = '2000 * (1 + x*x+y*y)'
  []
[]

[AuxVariables]
  [p]
    [InitialCondition]
      type = FunctionIC
      function = fn_1
    []
  []
  [h]
    [InitialCondition]
      type = FunctionIC
      function = fn_2
    []
  []

  [T]
    family = MONOMIAL
    order = CONSTANT
  []
  [s]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [T]
    type = MaterialRealAux
     variable = T
     property = T
  []
  [s]
    type = MaterialRealAux
     variable = s
     property = s
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
    type = FluidPropertiesMaterialPH
    pressure = p
    h = h
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
