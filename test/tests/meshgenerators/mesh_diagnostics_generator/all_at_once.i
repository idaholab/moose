[Mesh]
  [base]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
  [diag]
    type = MeshDiagnosticsGenerator
    input = base
    examine_element_overlap = WARNING
    examine_element_types = WARNING
    examine_element_volumes = WARNING
    examine_non_conformality = WARNING
    examine_nonplanar_sides = INFO
    examine_sidesets_orientation = WARNING
    check_for_watertight_sidesets = WARNING
    check_for_watertight_nodesets = WARNING
    examine_non_matching_edges = WARNING
    search_for_adaptivity_nonconformality = WARNING
    check_local_jacobian = WARNING
  []
[]

[Outputs]
  exodus = true
[]
