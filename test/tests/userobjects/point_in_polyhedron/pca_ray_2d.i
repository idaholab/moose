# Framework point-in-polyhedron test on a closed 2D EDGE2 surface using only
# MooseTestApp objects. A closed elliptical curve is saved and handed to a
# BoundaryMeshBuilder. A PointInPolyhedronCheckUO with the default pca_ray
# backend classifies background element centroids via a SpatialUserObjectAux
# (1 inside/on, 0 outside). This exercises the ray-casting engine's 2D EDGE2
# path through the framework input layer.
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

[AuxVariables]
  [inside]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [inside]
    type = SpatialUserObjectAux
    variable = inside
    user_object = in_out_test
    execute_on = 'initial'
  []
[]

[UserObjects]
  [surface_builder]
    type = BoundaryMeshBuilder
    surface_mesh = surface_mesh
  []

  [in_out_test]
    type = PointInPolyhedronCheckUO
    builder = surface_builder
    point_containment_method = pca_ray
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
