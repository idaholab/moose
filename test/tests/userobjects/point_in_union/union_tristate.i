# Tri-state union test. Two signed half-space level sets are unioned; nodes along
# x deterministically realize each precedence case (INSIDE > ON > OUTSIDE):
#   funcA = x - 1  (inside where x < 1),  funcB = x - 2  (inside where x < 2)
# Nodes are at x = 0, 1, 2, 3:
#   x = 0 : A INSIDE , B INSIDE  -> INSIDE  (2)
#   x = 1 : A ON     , B INSIDE  -> INSIDE  (2)   (ON overridden by INSIDE)
#   x = 2 : A OUTSIDE, B ON      -> ON      (1)   (ON over OUTSIDE)
#   x = 3 : A OUTSIDE, B OUTSIDE -> OUTSIDE (0)
# SurfaceSidenessTestAux reports the raw tri-state (outside=0, on=1, inside=2), so
# the gold distinguishes ON from INSIDE (which spatialValue() would collapse).

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
    user_object = union
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
  [union]
    type = PointInUnionCheckUO
    providers = 'checkA checkB'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
