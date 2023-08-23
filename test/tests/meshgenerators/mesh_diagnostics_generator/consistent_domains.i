[Mesh]
  [checker]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    subdomain_ids = '1 2
                     2 1'
  []
  [inner]
    type = ParsedGenerateSideset
    input = checker
    combinatorial_geometry = 'x>0.49 & x<0.51'
    new_sideset_name = 'bad_one'
  []
  [diag]
    type = MeshDiagnosticsGenerator
    input = inner
    examine_sidesets_orientation = INFO
  []
[]

[Outputs]
  exodus = true
[]
