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
        boundary = 'bad_side'
    []
[]
[Outputs]
    exodus = true
[]
