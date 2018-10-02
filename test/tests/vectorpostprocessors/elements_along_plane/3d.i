[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 10
  # Our CSV diffs here depend on a fixed element id numbering
  allow_renumbering = false
  parallel_type = replicated
[]

[Problem]
  solve = false
[]

[VectorPostprocessors]
  [./elems]
    type = ElementsAlongPlane
    point = '0.525 0.525 0.0'
    normal = '1.0 1.0 0.0'
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
