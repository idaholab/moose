[Mesh]
  [load_stl]
    type = FileMeshGenerator
    # file = 'Cluster_34.stl' # OK
    # file = 'engraving.stl' #OK
    # file = 'hinge.stl' # some artefacts from shared normals, fixed with contiguous_assignments_only = true
    # file = 'part1.stl' # OK
    file = 'plane.stl' # lots of artefacts
    # Plane:
    # flood only once helps get rid of plenty of small domains appearing under wings
    # contiguous assignments helps avoid creating new subdomains off of everywhere
    # check neighbors help with front of the wings
  []
  [paint]
    type = SurfaceSubdomainsFromAllNormalsGenerator
    input = 'load_stl'
    normal_tol = 0.7
    contiguous_assignments_only = true
    check_painted_neighbor_normals = true
    flood_elements_once = true

    select_max_neighbor_element_subdomains = true
  []
  # [re_paint]
  #   type = SurfaceSubdomainsFromAllNormalsGenerator
  #   input = 'paint'
  #   normal_tol = 0.8
  #   flood_elements_once = true
  #   contiguous_assignments_only = true
  #   included_subdomains = '0'
  # []
  [diag]
    type = MeshDiagnosticsGenerator
    input = paint
    examine_element_overlap = WARNING
    examine_element_types = WARNING
    examine_element_volumes = WARNING
    examine_non_conformality = WARNING
    examine_nonplanar_sides = INFO
    examine_sidesets_orientation = WARNING
    check_for_watertight_sidesets = WARNING
    check_for_watertight_nodesets = WARNING
    search_for_adaptivity_nonconformality = WARNING
    check_local_jacobian = WARNING
  []
[]
