# A closed curve lies entirely within one coarse element, so every element node is outside while
# interior quadrature points are inside. The element must still be marked as intercepted.
radius = 0.3
n_seg = 32

[Problem]
  solve = false
[]

[Mesh]
  [surface]
    type = ParsedCurveGenerator
    x_formula = '0.5 + ${radius} * cos(t)'
    y_formula = '0.5 + ${radius} * sin(t)'
    section_bounding_t_values = '0 ${fparse 2*pi}'
    nums_segments = '${n_seg}'
    is_closed_loop = true
    save_with_name = surface
  []
  [domain]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
    subdomain_ids = 1
  []
  add_subdomain_ids = '2 3'
  final_generator = domain
[]

[UserObjects]
  [surface_builder]
    type = SBMSurfaceMeshBuilder
    check_watertightness = true
    surface_mesh = surface
  []
  [in_out_test]
    type = PointInPolyhedronCheckUO
    builder = surface_builder
  []
[]

[MeshModifiers]
  [intercepted]
    type = InterceptedElementModifier
    subdomain_id_inside = 1
    subdomain_id_outside = 2
    is_domain_inside_surface = true
    in_out_test = in_out_test
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
