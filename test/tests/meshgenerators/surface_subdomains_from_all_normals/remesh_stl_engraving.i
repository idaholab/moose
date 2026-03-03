[Mesh]
  [load_stl]
    type = FileMeshGenerator
    file = 'engraving.stl'
  []
  [paint]
    type = SurfaceSubdomainsFromAllNormalsGenerator
    input = 'load_stl'
    normal_tol = 0.3
    flipped_normal_tol = 0.01
    contiguous_assignments_only = true
    check_painted_neighbor_normals = false
    flood_elements_once = true
    fixed_normal = true
    allow_normal_flips = false

    show_info = true
    output = true
  []

  [re_paint_subdomains_with_holes]
    type = SurfaceSubdomainsFromAllNormalsGenerator
    input = 'paint'
    normal_tol = 0.01
    included_subdomains = '2 12 15 51'
    max_subdomain_size_centroids = "7.8 10 10 10"
    contiguous_assignments_only = true
    flood_elements_once = true
    fixed_normal = true
    allow_normal_flips = false
    # Minimum (no cuts) -> 94 subdomains
    # 5 -> 220 subdomains
    # 10 -> 195 subdomains
    # 15 -> 184 subdomains
    # 20 -> 176 subdomains
    # 30 -> 165 subdomains
    # 50 -> 152
    # 60 -> 146
    # 80 -> fails
    # custom -> 142
    show_info = true
    output = true
  []
  [merge_some]
    type = RenameBlockGenerator
    input = 're_paint_subdomains_with_holes'
    old_block = '113'# 108 109 105 106 98 107'
    new_block = '200'# 200 200 200 200 200 200'
  []
  [flip_bad_elements]
    type = OrientSurfaceMeshGenerator
    input = 'merge_some'
    included_subdomains = '5'
    normal = '0 0 -1'
    output = true
  []
  [re_mesh]
    type = SurfaceSubdomainsDelaunayRemesher
    input = 'flip_bad_elements'
    # max_edge_length = 12
    interpolate_boundaries = 20
    refine_boundaries = false
    desired_areas = 5
    verbose = true

    max_angle_deviation = 70
  []
  [diag]
    type = MeshDiagnosticsGenerator
    input = re_mesh
    examine_element_overlap = WARNING
    examine_element_types = WARNING
    examine_element_volumes = WARNING
    # No point continuing if there is a hole in the surface mesh
    examine_non_conformality = WARNING
    examine_nonplanar_sides = INFO
    # examine_sidesets_orientation = WARNING
    check_for_watertight_sidesets = WARNING
    check_for_watertight_nodesets = WARNING
    search_for_adaptivity_nonconformality = WARNING
    check_local_jacobian = WARNING
  []

  [clear_names]
    type = ParsedSubdomainMeshGenerator
    input = 'diag'
    expression = 'x > -1e5'
    block_id = '0'
    output = true
  []
  [fix_overlaps]
    type = MeshRepairGenerator
    input = 'clear_names'
    fix_node_overlap = true
    output = true
  []
  [mesh_volume]
    type = XYZDelaunayGenerator
    boundary = 'fix_overlaps'
    output_subdomain_name = 'vol'
    # stitching_algorithm = 'exhaustive'
    desired_volume = '10'
    # there are no holes in this mesh
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  exodus = true
[]

# To debug the surface normal
[AuxVariables]
  [n_x]
    family = MONOMIAL
    order = CONSTANT
    [AuxKernel]
      type = ElementNormalAux
      component = x
    []
  []
  [n_y]
    family = MONOMIAL
    order = CONSTANT
    [AuxKernel]
      type = ElementNormalAux
      component = y
    []
  []
  [n_z]
    family = MONOMIAL
    order = CONSTANT
    [AuxKernel]
      type = ElementNormalAux
      component = z
    []
  []
[]
