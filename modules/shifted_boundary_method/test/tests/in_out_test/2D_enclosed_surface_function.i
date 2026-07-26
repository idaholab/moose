# A level-set curve lies entirely within one coarse element, so every element node is outside while
# interior quadrature points are inside. The element must still be marked as intercepted.
radius = 0.3

[Problem]
  solve = false
[]

[Mesh]
  [domain]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
    subdomain_ids = 1
  []
  add_subdomain_ids = '2 3'
[]

[Functions]
  [level_set]
    type = ParsedFunction
    expression = '(x - 0.5)^2 + (y - 0.5)^2 - ${fparse radius^2}'
  []
[]

[MeshModifiers]
  [intercepted]
    type = InterceptedElementModifier
    subdomain_id_inside = 1
    subdomain_id_outside = 2
    is_domain_inside_surface = true
    signed_dist_function = level_set
    mark_intercepted = true
    subdomain_id_intercepted = 3
    execute_on = INITIAL
  []
[]

[Variables]
  [u]
    initial_condition = 1
    block = 3
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
