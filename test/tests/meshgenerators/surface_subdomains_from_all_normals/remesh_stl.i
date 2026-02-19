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
    normal_tol = 0.3
    contiguous_assignments_only = true
    check_painted_neighbor_normals = false
    flood_elements_once = true
    fixed_normal = true

    # only used for the plan STL
    # select_max_neighbor_element_subdomains = true
    show_info = true
    output = true
  []
  [flip_bad_elements]
    type = MeshRepairGenerator
    input = 'paint'
    fix_elements_orientation = true
  []
  [re_mesh]
    type = SurfaceSubdomainsDelaunayRemesher
    input = 'flip_bad_elements'
    # 1,12,15 have holes (front frace, letter e and b)
    exclude_subdomain_names = '2 12 15'
    # subdomain_names = '1;3;4;5;6;7;8;9;10;11;13;14;16;17;18;19;20;21;22;23;24;25;26;27;28;29;30;31;32;33;34;35;36;37;38;39;40;41;42;43;44;45;46;47;48;49;50;51;52;53;54;55;56;57;58'
    # subdomain_names = '0; 1; 2; 3; 4; 5; 6; 7; 8; 9; 10; 11; 12'
    max_edge_length = 0.1
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
