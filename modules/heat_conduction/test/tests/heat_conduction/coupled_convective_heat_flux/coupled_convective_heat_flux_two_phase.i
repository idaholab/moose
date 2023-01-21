[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Functions]
  [./alpha_liquid_fn]
    type = ParsedFunction
    expression = 'sin(pi*y)'
  [../]
  [./T_infinity_liquid_fn]
    type = ParsedFunction
    expression = '(x*x+y*y)+500'
  [../]
  [./Hw_liquid_fn]
    type = ParsedFunction
    expression = '((1-x)*(1-x)+(1-y)*(1-y))+1000'
  [../]

  [./alpha_vapor_fn]
    type = ParsedFunction
    expression = '1-sin(pi*y)'
  [../]
  [./T_infinity_vapor_fn]
    type = ParsedFunction
    expression = '(x*x+y*y)+5'
  [../]
  [./Hw_vapor_fn]
    type = ParsedFunction
    expression = '((1-x)*(1-x)+(1-y)*(1-y))+10'
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./T_infinity_liquid]
  [../]
  [./Hw_liquid]
  [../]
  [./alpha_liquid]
  [../]
  [./T_infinity_vapor]
  [../]
  [./Hw_vapor]
  [../]
  [./alpha_vapor]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./force]
    type = BodyForce
    variable = u
    value = 1000
  [../]
[]

[AuxKernels]
  [./alpha_liquid_ak]
    type = FunctionAux
    variable = alpha_liquid
    function = alpha_liquid_fn
    execute_on = initial
  [../]
  [./T_infinity_liquid_ak]
    type = FunctionAux
    variable = T_infinity_liquid
    function = T_infinity_liquid_fn
    execute_on = initial
  [../]
  [./Hw_liquid_ak]
    type = FunctionAux
    variable = Hw_liquid
    function = Hw_liquid_fn
    execute_on = initial
  [../]

  [./alpha_vapor_ak]
    type = FunctionAux
    variable = alpha_vapor
    function = alpha_vapor_fn
    execute_on = initial
  [../]
  [./T_infinity_vapor_ak]
    type = FunctionAux
    variable = T_infinity_vapor
    function = T_infinity_vapor_fn
    execute_on = initial
  [../]
  [./Hw_vapor_ak]
    type = FunctionAux
    variable = Hw_vapor
    function = Hw_vapor_fn
    execute_on = initial
  [../]
[]

[BCs]
  [./right]
    type = CoupledConvectiveHeatFluxBC
    variable = u
    boundary = right
    alpha = 'alpha_liquid alpha_vapor'
    htc = 'Hw_liquid Hw_vapor'
    T_infinity = 'T_infinity_liquid T_infinity_vapor'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
