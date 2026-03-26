[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 3
    dx = '1.5 2.4 0.1'
    dy = '1.3 0.9'
    dz = '0.4 0.5 0.6 0.7'
    # avoid rounding error, we want the ix & iy to match the ix/iy test input
    # Note: we do rely on 0.6/0.6 = 1. There should be no rounding error on that.
    max_sizes_in_x = '0.8 2.4 0.1'
    max_sizes_in_y = '0.7 0.31'
    max_sizes_in_z = '0.4 0.5 0.6 0.7'
    subdomain_id = '0 1 1
                    2 2 2

                    3 4 4
                    5 5 5

                    0 1 1
                    2 2 2

                    3 4 4
                    5 5 5
                    '
  []
[]
