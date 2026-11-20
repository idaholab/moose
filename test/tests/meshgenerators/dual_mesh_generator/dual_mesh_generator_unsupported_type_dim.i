[Mesh]
    [mySphere]
        type = SphereMeshGenerator
        nr = 1
        radius = 1
    []

    [myDualGen]
        type = DualMeshGenerator
        input = mySphere
        dual_mesh_type = voronoi
    []

    [convert]
        type = ElementsToSimplicesConverter
        input = myDualGen
    []
[]
