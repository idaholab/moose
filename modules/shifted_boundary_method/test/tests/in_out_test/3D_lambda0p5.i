nx = 16
lambda = 0.5
# Immersed triaxial ellipsoid: three distinct semi-axes make the boundary
# anisotropic so PCA has a well-separated shortest axis, giving a reproducible
# auto ray direction. Off-grid center keeps the surface cutting elements at
# generic fractions with no node/quadrature point landing exactly on it.
ax = 1.5
ay = 1.15
az = 0.85
cx = 2.03
cy = 1.97
cz = 2.05
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
  [scale]
    type = TransformGenerator
    transform = SCALE
    vector_value = '${ax} ${ay} ${az}'
    input = extract
  []
  [shift_boundary_mesh]
    type = TransformGenerator
    transform = TRANSLATE
    vector_value = '${cx} ${cy} ${cz}'
    input = scale
    save_with_name = 'shift_boundary_mesh'
  []

  [gen]
    type = CartesianMeshGenerator
    dim = 3
    dx = '4'
    dy = '4'
    dz = '4'
    ix = '${nx}'
    iy = '${nx}'
    iz = '${nx}'
    subdomain_id = '1'
  []

  add_subdomain_ids = 2 # outside block
  add_sideset_names = 'SBMinterface'
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
