[Mesh]
    [myCartMG]
        type = CartesianMeshGenerator
        dim = 3
        dx = '3 3'
        dy = '4 4'
        dz = '5 5'
    []

    [myDualGen]
        type = DualMeshGenerator
        input = myCartMG

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