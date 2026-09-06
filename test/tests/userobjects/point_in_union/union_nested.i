# Nested union test. A union is itself a provider of another union:
#   inner = union(funcA = x - 1, funcB = x - 2)
#   outer = union(inner, funcC = x - 2.5)
# Nodes are at x = 0, 1, 2, 3:
#   x = 0 : inner INSIDE                      -> outer INSIDE  (2)
#   x = 1 : inner INSIDE                      -> outer INSIDE  (2)
#   x = 2 : inner ON , C INSIDE (2 < 2.5)     -> outer INSIDE  (2)  (nested ON beaten by sibling INSIDE)
#   x = 3 : inner OUTSIDE, C OUTSIDE          -> outer OUTSIDE (0)
# Proves the documented nesting and that precedence flows through nested unions.

[Problem]
  solve = false
[]

[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '3'
    dy = '1'
    ix = '3'
    iy = '1'
    subdomain_id = '1'
  []
[]

[Functions]
  [funcA]
    type = ParsedFunction
    expression = 'x - 1'
  []
  [funcB]
    type = ParsedFunction
    expression = 'x - 2'
  []
  [funcC]
    type = ParsedFunction
    expression = 'x - 2.5'
  []
[]

[AuxVariables]
  [sideness]
    family = LAGRANGE
    order = FIRST
  []
[]

[AuxKernels]
  [sideness]
    type = SurfaceSidenessTestAux
    variable = sideness
    user_object = outer
    execute_on = 'initial'
  []
[]

[UserObjects]
  [checkA]
    type = PointInSignedFunctionCheckUO
    function = funcA
  []
  [checkB]
    type = PointInSignedFunctionCheckUO
    function = funcB
  []
  [checkC]
    type = PointInSignedFunctionCheckUO
    function = funcC
  []
  [inner]
    type = PointInUnionCheckUO
    providers = 'checkA checkB'
  []
  [outer]
    type = PointInUnionCheckUO
    providers = 'inner checkC'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
