# Same immersed ellipse as 2D_lambda0p5.i, but with mark_intercepted = true so
# elements the surface cuts through are assigned a dedicated subdomain
# (subdomain_id_intercepted) instead of being resolved by the lambda fraction.
# This exercises the mark_intercepted return path in
# InterceptedElementModifier::computeSubdomainID and the corresponding branch of
# SBMElementSubdomainModifierBase.
nx = 16
lambda = 0.5
a = 1.6
b = 1.0
cx = 2.03
cy = 1.97
n_seg = 48
[Problem]
  solve = false
[]

[Mesh]
  [shift_boundary_mesh]
    type = ParsedCurveGenerator
    x_formula = '${cx} + ${a} * cos(t)'
    y_formula = '${cy} + ${b} * sin(t)'
    section_bounding_t_values = '0 ${fparse 2*pi}'
    nums_segments = '${n_seg}'
    is_closed_loop = true
    save_with_name = 'shift_boundary_mesh'
  []

  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '4'
    dy = '4'
    ix = '${nx}'
    iy = '${nx}'
    subdomain_id = '1'
  []

  add_subdomain_ids = '2 3' # outside block and intercepted block
  final_generator = 'gen'
[]

[UserObjects]
  [TreeBuilder]
    type = SBMSurfaceMeshBuilder
    check_watertightness = true
    surface_mesh = shift_boundary_mesh
  []

  [inout_test]
    type = PointInPolyhedronCheckUO
    brute_force = true
    builder = TreeBuilder
  []
[]

[MeshModifiers]
  [IntercpetedESM]
    type = InterceptedElementModifier
    subdomain_id_inside = 1
    subdomain_id_outside = 2
    lambda = ${lambda}
    outer_boundary = true
    in_out_test = inout_test
    mark_intercepted = true
    subdomain_id_intercepted = 3
    execute_on = 'INITIAL'
    execution_order_group = 0
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
