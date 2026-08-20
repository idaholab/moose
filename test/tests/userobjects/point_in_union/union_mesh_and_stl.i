# Three-dimensional union of two closed TRI3 surfaces given in mixed
# representations: a generator-built sphere shell (the boundary of a TET4 sphere)
# and a tetrahedron read from tetrahedron.stl. A point is contained when it is
# inside either surface.

[Problem]
  solve = false
[]

[Mesh]
  # Generator-built closed surface: extract the outer TRI3 shell of a TET4 sphere
  # and translate it to sit well above the STL tetrahedron, leaving a clear gap so
  # the two contained regions are visually distinct.
  [ball]
    type = SphereMeshGenerator
    radius = 1.0
    nr = 2
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
  [sphere_surf]
    type = TransformGenerator
    transform = TRANSLATE
    vector_value = '2.0 2.0 4.5'
    input = extract
    save_with_name = 'sphere_surf'
  []

  # Closed surface read from an STL file (a tetrahedron spanning z in [1, 3]).
  [tet_surf]
    type = FileMeshGenerator
    file = tetrahedron.stl
    save_with_name = 'tet_surf'
  []

  [gen]
    type = CartesianMeshGenerator
    dim = 3
    dx = '4'
    dy = '4'
    dz = '6'
    ix = '8'
    iy = '8'
    iz = '12'
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
    user_object = union
    execute_on = 'initial'
  []
[]

[UserObjects]
  [builder1]
    type = BoundaryMeshBuilder
    surface_mesh = sphere_surf
  []
  [builder2]
    type = BoundaryMeshBuilder
    surface_mesh = tet_surf
  []
  [check1]
    type = PointInPolyhedronCheckUO
    builder = builder1
    point_containment_method = pca_ray
  []
  [check2]
    type = PointInPolyhedronCheckUO
    builder = builder2
    point_containment_method = pca_ray
  []
  [union]
    type = PointInUnionCheckUO
    providers = 'check1 check2'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
