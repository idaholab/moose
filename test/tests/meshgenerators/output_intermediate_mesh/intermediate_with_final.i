[Mesh]
  [final]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 2
  []

  [also_output]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 3
    output = true
  []

  final_generator = final
[]

