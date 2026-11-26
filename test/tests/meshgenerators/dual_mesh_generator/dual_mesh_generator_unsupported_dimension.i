# dual_mesh_generator_unsupported_dimension.i
[Mesh]
    [input]
        type = GeneratedMeshGenerator
        dim = 1
        nx = 2
        xmax = 1
    []

    [dual]
        type = DualMeshGenerator
        input = input
    []
[]
