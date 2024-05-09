[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
  []
[]

[Functions]
  [functor]
    type = ParsedFunction
    expression = x+y^2
  []
[]

[AuxVariables]
  [u]
    family = MONOMIAL
    order = CONSTANT
  []
  [v]
    family = LAGRANGE
    order = FIRST
  []
[]

[AuxKernels]
  [u]
    type = FunctorMaterialRealAux
    variable = u
    functor = functor
  []
  [v]
    type = FunctorMaterialRealAux
    variable = v
    functor = functor
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  execute_on = FINAL
[]
