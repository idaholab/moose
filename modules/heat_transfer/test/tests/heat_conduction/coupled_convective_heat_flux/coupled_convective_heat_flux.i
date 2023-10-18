[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Functions]
  [./T_infinity_fn]
    type = ParsedFunction
    expression = (x*x+y*y)+500
  [../]
  [./Hw_fn]
    type = ParsedFunction
    expression = ((1-x)*(1-x)+(1-y)*(1-y))+1000
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./T_infinity]
  [../]
  [./Hw]
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
  [./T_infinity_ak]
    type = FunctionAux
    variable = T_infinity
    function = T_infinity_fn
    execute_on = initial
  [../]
  [./Hw_ak]
    type = FunctionAux
    variable = Hw
    function = Hw_fn
    execute_on = initial
  [../]
[]

[BCs]
  [./right]
    type = CoupledConvectiveHeatFluxBC
    variable = u
    boundary = right
    htc = Hw
    T_infinity = T_infinity
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
