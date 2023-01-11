[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  elem_type = QUAD4
[]

[Functions]
  [./f_fn]
    type = ParsedFunction
    expression = -4
  [../]
  [./bc_fn]
    type = ParsedFunction
    expression = 'x*x+y*y'
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./e]
    initial_condition = 6232.5
  [../]
  [./v]
    initial_condition = 0.02493
  [../]

  [./p]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./T]
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
  [./g]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxKernels]
  [./p]
    type = MaterialRealAux
     variable = p
     property = pressure
  [../]
  [./T]
    type = MaterialRealAux
     variable = T
     property = temperature
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
     property = mu
  [../]
  [./k]
    type = MaterialRealAux
     variable = k
     property = k
  [../]
  [./g]
    type = MaterialRealAux
     variable = g
     property = g
  [../]
[]

[FluidProperties]
  [./ideal_gas]
    type = IdealGasFluidProperties
    gamma = 1.4
    molar_mass = 1.000536678700361
  [../]
[]

[Materials]
  [./fp_mat]
    type = FluidPropertiesMaterialVE
    e = e
    v = v
    fp = ideal_gas
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./ffn]
    type = BodyForce
    variable = u
    function = f_fn
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = 'left right top bottom'
    function = bc_fn
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
