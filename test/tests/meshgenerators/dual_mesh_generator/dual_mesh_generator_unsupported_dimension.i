[Mesh]
    [1d_mesh]
        type = GeneratedMeshGenerator
        dim = 1
        nx = 2
        xmax = 1
    []

    [myDualGen]
        type = DualMeshGenerator
        input = 1d_mesh
    []

    [convert]
        type = ElementsToSimplicesConverter
        input = 'myDualGen'
    []
[]
