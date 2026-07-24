# Same immersed ellipse as 2D_lambda0p5.i, but the InterceptedElementModifier is
# driven by a signed level-set function (signed_dist_function) instead of a
# geometric in-out UserObject. This exercises the SIGN_DISTANCE branch of
# InterceptedElementModifier::computeSubdomainID. The level set is negative
# inside the ellipse and positive outside, matching outer_boundary = true.
nx = 16
lambda = 0.5
a = 1.6
b = 1.0
cx = 2.03
cy = 1.97
[Problem]
  solve = false
[]

[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '4'
    dy = '4'
    ix = '${nx}'
    iy = '${nx}'
    subdomain_id = '1'
  []

  add_subdomain_ids = 2 # outside block
  add_sideset_names = 'SBMinterface'
  final_generator = 'gen'
[]

[Functions]
  [level_set]
    type = ParsedFunction
    expression = '((x - ${cx}) / ${a})^2 + ((y - ${cy}) / ${b})^2 - 1'
  []
[]

[MeshModifiers]
  [IntercpetedESM]
    type = InterceptedElementModifier
    subdomain_id_inside = 1
    subdomain_id_outside = 2
    lambda = ${lambda}
    outer_boundary = true
    signed_dist_function = level_set
    execute_on = 'INITIAL'
    execution_order_group = 0
  []
  [SBMinterface]
    type = SidesetAroundSubdomainUpdater
    inner_subdomains = 1
    outer_subdomains = 2
    update_sideset_name = 'SBMinterface'
    assign_outer_surface_sides = false
    execute_on = 'INITIAL'
    execution_order_group = 1
    block = '1 2' # need to override default block
  []
[]

[Variables]
  [u]
    initial_condition = 1
    block = 1
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
