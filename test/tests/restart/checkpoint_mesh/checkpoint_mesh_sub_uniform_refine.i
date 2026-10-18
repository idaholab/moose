[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  uniform_refine = 1
[]

[Postprocessors]
  [num_elems]
    type = NumElements
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
