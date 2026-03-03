[Mesh]
  [load_stl]
    type = FileMeshGenerator
    file = 'hinge.stl'
  []
  [paint]
    type = SurfaceSubdomainsFromAllNormalsGenerator
    input = 'load_stl'
    normal_tol = 0.1
    contiguous_assignments_only = true
    check_painted_neighbor_normals = false
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
    # all the subdomains with holes
    # 54 and 60 are top and bottom exterior
    # 52 and 1 are "door" facing and outwards
    included_subdomains = '83 92 82 79 94 103'
    contiguous_assignments_only = true
    flood_elements_once = true
    fixed_normal = true
    max_subdomain_size_centroids = '15 15 6 6 7 7'
    show_info = true
    output = true
  []
  # [flip_bad_elements]
  #   type = OrientSurfaceMeshGenerator
  #   input = 're_paint_subdomains_with_holes'
  #   included_subdomains = '117'
  #   normal = '0 0 -1'
  #   output = true
  # []
  [re_mesh]
    type = SurfaceSubdomainsDelaunayRemesher
    input = 're_paint_subdomains_with_holes'
    interpolate_boundaries = 2
    refine_boundaries = false
    desired_areas = 1
    verbose = true
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
    # large desired volume can create holes in the mesh
    desired_volume = '10'
    # there are no holes in this mesh
  []

  [Partitioner]
    type = GridPartitioner
    grid_computation = 'manual'
    nx = 10
    ny = 1
    nz = 1
  []
[]
