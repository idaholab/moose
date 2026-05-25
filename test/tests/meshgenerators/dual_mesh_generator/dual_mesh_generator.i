[Mesh]
    [myCartMG]
        type = CartesianMeshGenerator
        dim = 2
        dx = '3 3'
        dy = '4 4'
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