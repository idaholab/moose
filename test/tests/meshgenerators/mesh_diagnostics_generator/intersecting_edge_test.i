[Mesh]
  [gmg1]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
    xmin = -0.5
    xmax = 0.5
    ymin = -0.5
    ymax = 0.5
    zmin = -1.5
    zmax = -0.5
  []
  [gmg2]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
    xmin = -0.5
    xmax = 0.5
    ymin = -0.5
    ymax = 0.5
    zmin = -0.5
    zmax = 0.5
  []
  [block_1]
    type = ParsedSubdomainMeshGenerator
    input = gmg1
    combinatorial_geometry = 'z < -0.5'
    block_id = 1
  []
  [block_2]
    type = ParsedSubdomainMeshGenerator
    input = gmg2
    combinatorial_geometry = 'z > 0.5'
    block_id = 2
  []
  [convert1]
    type = ElementsToTetrahedronsConverter
    input = 'block_1'
  []
  [convert2]
    type = ElementsToTetrahedronsConverter
    input = 'block_2'
  []
  [rotate]
    type = TransformGenerator
    input = convert2
    transform = 'rotate'
    vector_value = '180 0 0'
  []
  [stitch]
    type = StitchedMeshGenerator
    inputs = 'convert1 rotate'
    stitch_boundaries_pairs = 'front back'
  []
  [diag]
    type = MeshDiagnosticsGenerator
    input = stitch
    examine_non_matching_edges = INFO
  []
[]
[Outputs]
  exodus = true
[]
