[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Problem]
  type = DumpObjectsProblem
  kernel_coverage_check = false
  dump_path = 'DumpTest/test'
[]

[DumpTest]
  [test]
  []
[]

[Executioner]
  type = Transient
[]
