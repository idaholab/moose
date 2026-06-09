[Mesh]
    [myCartMG]
        type = CartesianMeshGenerator
        dim = 2
        dx = "3 3 3"
        dy = "3 3 3"

        subdomain_id = '4 8 3
                        2 5 6
                        7 9 10'

    []

    [cut_1]
        type = BlockDeletionGenerator
        input = myCartMG
        block = 2
    []

    [cut_2]
        type = BlockDeletionGenerator
        input = cut_1
        block = 5
    []

    [myDualGen]
        type = DualMeshGenerator
        input = cut_2
    []

    [SdmPerElemGen]
        type = SubdomainPerElementGenerator
        input = myDualGen
    []

    [convert]
        type = ElementsToSimplicesConverter
        input = 'SdmPerElemGen'
    []


[]