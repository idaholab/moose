# The fixed_x_ray backend (TriangleManifold engine) supports TRI3 surfaces only.
# Here the saved surface is the QUAD4 boundary of a hex cube (mesh_dimension 2 but
# not TRI3), so PointInPolyhedronCheckUO must reject it up front with a
# method-aware message rather than failing later inside TriangleManifold.
[Problem]
  solve = false
[]

[Mesh]
  [cube]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
    xmin = 1
    xmax = 3
    ymin = 1
    ymax = 3
    zmin = 1
    zmax = 3
  []
  [shell]
    type = LowerDBlockFromSidesetGenerator
    input = cube
    sidesets = 'left right bottom top back front'
    new_block_id = 100
  []
  [surface_mesh]
    type = BlockToMeshConverterGenerator
    input = shell
    target_blocks = '100'
    save_with_name = 'surface_mesh'
  []

  [gen]
    type = CartesianMeshGenerator
    dim = 3
    dx = '4'
    dy = '4'
    dz = '4'
    ix = '4'
    iy = '4'
    iz = '4'
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
    point_containment_method = fixed_x_ray
  []
[]

[Executioner]
  type = Steady
[]
