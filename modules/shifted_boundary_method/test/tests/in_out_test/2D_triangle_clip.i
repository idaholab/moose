# Test a triangular closed surface with sharp corners and intercepted elements.

[Problem]
  solve = false
[]

[Mesh]
  [surface]
    type = ParsedCurveGenerator
    # Piecewise-linear triangle A->B->C; is_closed_loop adds the closing edge C->A.
    # Apex A sits at a cell center (0.5625, 0.8125) with a narrow wedge, so the apex cell has all
    # four nodes outside the triangle while a finite sliver is inside.
    constant_names = 'Ax Ay Bx By Cx Cy'
    constant_expressions = '0.5625 0.8125 0.30 0.20 0.72 0.20'
    x_formula = 'if(t < 1, Ax + t * (Bx - Ax), Bx + (t - 1) * (Cx - Bx))'
    y_formula = 'if(t < 1, Ay + t * (By - Ay), By + (t - 1) * (Cy - By))'
    section_bounding_t_values = '0 1 2'
    nums_segments = '10 10'
    is_closed_loop = true
    save_with_name = surface
  []
  [domain]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 8
    ny = 8
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
