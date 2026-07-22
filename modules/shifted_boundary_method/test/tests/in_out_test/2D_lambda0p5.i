nx = 16
lambda = 0.5
# Immersed circle geometry: off-grid center so the arc cuts through elements at
# generic fractions and no node/quadrature point lands exactly on the surface.
R = 1.3
cx = 2.03
cy = 1.97
n_seg = 48
[Problem]
  solve = false
[]

[Mesh]
  [shift_boundary_mesh]
    type = ParsedCurveGenerator
    x_formula = '${cx} + ${R} * cos(t)'
    y_formula = '${cy} + ${R} * sin(t)'
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
