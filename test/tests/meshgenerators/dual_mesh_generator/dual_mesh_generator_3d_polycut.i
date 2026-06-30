[Mesh]
    [myCCMG]
        type = CartesianMeshGenerator
        dim = 3
        dx = '3 3 3'
        dy = '4 4 4'
        dz = '5 5 5'
        subdomain_id = '1 2 3
                        4 5 6
                        7 8 9

                        10 11 12
                        13 14 15
                        16 17 18

                        19 20 21
                        22 23 24
                        25 26 27'
    []

    [cut]
        type = BlockDeletionGenerator
        input = myCCMG
        block = '25 26 27'
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
