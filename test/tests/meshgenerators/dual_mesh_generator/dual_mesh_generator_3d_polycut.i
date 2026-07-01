[Mesh]
    [myCCMG]
        type = CartesianMeshGenerator
        dim = 3
        dx = '3 3'
        dy = '4 4'
        dz = '5 5'
        subdomain_id = '1 2
                        3 4

                        5 6
                        7 8'
    []

    [cut]
        type = BlockDeletionGenerator
        input = myCCMG
        block = '5 6'
    []

    [myDualGen]
        type = DualMeshGenerator
        input = cut
        dual_mesh_type = barycentric
        concave_treatment = 'split polycut netgen'
    []

    [convert]
        type = ElementsToSimplicesConverter
        input = myDualGen
    []
[]
