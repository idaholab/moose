[Mesh]
    [sphere]
        type = SphereMeshGenerator
        nr = 2
        radius = 4
    []

    [myDualGen]
        type = DualMeshGenerator
        input = sphere
        dual_mesh_type = barycentric
        concave_treatment = 'split polycut netgen'
    []

    [convert]
        type = ElementsToSimplicesConverter
        input = myDualGen
    []
[]
