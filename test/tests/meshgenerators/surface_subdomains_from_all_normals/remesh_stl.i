[Mesh]
  [load_stl]
    type = FileMeshGenerator
    # file = 'Cluster_34.stl' # OK, can remesh 1 by 1
    file = 'engraving.stl' #OK
    # file = 'hinge.stl' # some artefacts from shared normals, fixed with contiguous_assignments_only = true
    # file = 'part1.stl' # OK
    # file = 'plane.stl' # lots of artefacts
    # Plane:
    # flood only once helps get rid of plenty of small domains appearing under wings
    # contiguous assignments helps avoid creating new subdomains off of everywhere
    # check neighbors help with front of the wings
  []
  [paint]
    type = SurfaceSubdomainsFromAllNormalsGenerator
    input = 'load_stl'
    normal_tol = 0.8
    contiguous_assignments_only = true
    check_painted_neighbor_normals = false
    flood_elements_once = true

    # only used for the plan STL
    # select_max_neighbor_element_subdomains = true
    show_info = true
    output = true
  []
  [add_subd]
    type = RenameBlockGenerator
    input = 'paint'
    old_block = '1 2 3 4 5 6 7 8 9 10'
    new_block = 'a b c d e f g h i j'
  []
  [flip_bad_elements]
    type = MeshRepairGenerator
    input = 'add_subd'
    fix_elements_orientation = true
  []
  [re_mesh]
    type = SurfaceSubdomainsDelaunayRemesher
    input = 'flip_bad_elements'
    subdomain_names = '1; 2; 3; 4; 5; 6; 7; 8; 9; 10'
    # subdomain_names = '0; 1; 2; 3; 4; 5; 6; 7; 8; 9; 10; 11; 12'
    verbose = true
  []
  [diag]
    type = MeshDiagnosticsGenerator
    input = re_mesh
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
