# Framework point-in-polyhedron test using only MooseTestApp objects.
#
# A closed TRI3 surface (the shell of a TET4 sphere) is saved and handed to a
# BoundaryMeshBuilder. A PointInPolyhedronCheckUO with the fixed_x_ray backend
# (the TriangleManifold engine) classifies element centroids of the background
# mesh via a SpatialUserObjectAux (1 inside/on, 0 outside).
cx = 2.0
cy = 2.0
cz = 2.0

[Problem]
  solve = false
[]

[Mesh]
  [ball]
    type = SphereMeshGenerator
    radius = 1.0
    nr = 1
    elem_type = TET4
  []
  [shell]
    type = LowerDBlockFromSidesetGenerator
    input = ball
    sidesets = '0'
    new_block_id = 100
  []
  [extract]
    type = BlockToMeshConverterGenerator
    input = shell
    target_blocks = '100'
  []
  [surface_mesh]
    type = TransformGenerator
    transform = TRANSLATE
    vector_value = '${cx} ${cy} ${cz}'
    input = extract
    save_with_name = 'surface_mesh'
  []

  [gen]
    type = CartesianMeshGenerator
    dim = 3
    dx = '4'
    dy = '4'
    dz = '4'
    ix = '8'
    iy = '8'
    iz = '8'
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
    point_containment_method = fixed_x_ray
    # fixed_x_ray uses the TriangleManifold engine, which does not emit OBB/ray
    # debug files; supplying these names must be ignored (info message only).
    obb_file_name = 'should_not_be_written_obb.e'
    ray_file_name = 'should_not_be_written_ray.e'
  []
[]

[Executioner]
  type = Steady
[]
