[Mesh]
    [myCartMG]
        type = CartesianMeshGenerator
        dim = 2
        dx = "3 3"
        dy = "4 4"

        subdomain_id = '2 6
                        4 8'

    []

    [cut_corner]
        type = BlockDeletionGenerator
        input = myCartMG
        block = 2
    []
    
    [myDualGen]
        type = DualMeshGenerator
        input = cut_corner
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