[Mesh]
  [load_stl]
    type = FileMeshGenerator
    file = 'part1.stl'
  []
  [paint]
    type = SurfaceSubdomainsFromAllNormalsGenerator
    input = 'load_stl'
    normal_tol = 0.3

    # We need contiguous subdomain patches, and we cannot paint surfaces
    # with too large of an angle so the normal should be fixed
    contiguous_assignments_only = true
    flood_elements_once = true
    fixed_normal = true

    show_info = true
    output = true
  []

  [re_paint_subdomains_with_holes]
    type = SurfaceSubdomainsFromAllNormalsGenerator
    input = 'paint'
    # we need a tight tolerance to force different subdomains on the slope break
    normal_tol = 0.001

    contiguous_assignments_only = true
    flood_elements_once = true
    fixed_normal = true

    # all the subdomains with the central hole + the slope break
    included_subdomains = '1 16 20 21'
    # prevent 2D holes by cutting the subdomains with a certain max size
    max_subdomain_size_centroids = '1 10 3 3'

    show_info = true
    output = true
  []
  [re_mesh]
    type = SurfaceSubdomainsDelaunayRemesher
    input = 're_paint_subdomains_with_holes'

    # Triangulation parameters
    interpolate_boundaries = 2
    refine_boundaries = false
    desired_areas = 0.1

    verbose = true
  []
  [diag]
    type = MeshDiagnosticsGenerator
    input = re_mesh
    examine_element_overlap = WARNING
    examine_element_types = WARNING
    examine_element_volumes = WARNING
    # No point continuing if there is a hole in the surface mesh
    examine_non_conformality = ERROR
    examine_nonplanar_sides = INFO
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
  [mesh_volume]
    type = XYZDelaunayGenerator
    boundary = 'clear_names'
    output_subdomain_name = 'vol'
    # large desired volume can create holes in the mesh
    desired_volume = '10'
    # there are no holes in this mesh
  []
[]
