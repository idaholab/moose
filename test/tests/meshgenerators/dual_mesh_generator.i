[Mesh]
    [myCartMG]
        type = CartesianMeshGenerator
        dim = 2
        dx = '3 3'
        dy = '4 4'
    []

    [myDualGen]
        type = DualMeshGenerator
        block_id = 0
        bottom_left = '0 0 0'
        input = myCartMG
        top_right = '6 8 0'
    []

    [convert]
        type = ElementsToSimplicesConverter
        input = 'myDualGen'
    []
[]