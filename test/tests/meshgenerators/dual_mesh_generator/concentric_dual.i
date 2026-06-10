[Mesh]
    [myCCMG]
        type = ConcentricCircleMeshGenerator

        num_sectors = 2
        radii = '2'
        rings = '2'
        has_outer_square = false
        preserve_volumes = false
    []
    
    [myDualGen]
        type = DualMeshGenerator
        input = myCCMG
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