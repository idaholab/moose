# Framework point-in-polyhedron test proving the user_selected_ray method uses the caller's
# ray_direction exactly, rather than silently replacing it with the PCA auto direction. The
# anisotropic ellipse (a > b) makes the PCA direction ~ the y axis, so a user x-direction is
# discriminating: if the direction were swapped for PCA, the reported direction would differ.
# A PointInPolyhedronRayDirectionPostprocessor (test-only) reports the resolved ray direction
# components to CSV.
a = 1.6
b = 1.0
cx = 2.03
cy = 1.97

[Problem]
  solve = false
[]

[Mesh]
  [surface_mesh]
    type = ParsedCurveGenerator
    x_formula = '${cx} + ${a} * cos(t)'
    y_formula = '${cy} + ${b} * sin(t)'
    section_bounding_t_values = '0 ${fparse 2*pi}'
    nums_segments = '48'
    is_closed_loop = true
    save_with_name = 'surface_mesh'
  []

  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '4'
    dy = '4'
    ix = '16'
    iy = '16'
    subdomain_id = '1'
  []

  final_generator = 'gen'
[]

[UserObjects]
  [surface_builder]
    type = BoundaryMeshBuilder
    surface_mesh = surface_mesh
  []

  [in_out_test]
    type = PointInPolyhedronCheckUO
    builder = surface_builder
    point_containment_method = user_selected_ray
    ray_direction = '1 0 0' # overridden per test via cli_args
  []
[]

[Postprocessors]
  [ray_x]
    type = PointInPolyhedronRayDirectionPostprocessor
    user_object = in_out_test
    component = x
    execute_on = 'initial'
  []
  [ray_y]
    type = PointInPolyhedronRayDirectionPostprocessor
    user_object = in_out_test
    component = y
    execute_on = 'initial'
  []
  [ray_z]
    type = PointInPolyhedronRayDirectionPostprocessor
    user_object = in_out_test
    component = z
    execute_on = 'initial'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  execute_on = 'initial'
[]
