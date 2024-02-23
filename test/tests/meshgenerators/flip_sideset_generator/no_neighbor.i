[Mesh]
    [gmg]
        type = GeneratedMeshGenerator
        dim = 2
        nx = 1
        ny = 1
    []
    [flip]
        type = FlipSidesetGenerator
        input = gmg
        boundary = 'right'
    []
[]
[Outputs]
    exodus = true
[]
