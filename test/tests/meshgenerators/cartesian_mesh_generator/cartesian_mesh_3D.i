[Mesh]
  [./cmg]
    type = CartesianMeshGenerator
    dim = 3
    dx = '1.5 2.4 0.1'
    dy = '1.3 0.9'
    dz = '0.4 0.5 0.6 0.7'
    ix = '2 1 1'
    iy = '2 3'
    iz = '1 1 1 1'
    subdomain_id = '0 1 1
                    2 2 2

                    3 4 4
                    5 5 5

                    0 1 1
                    2 2 2

                    3 4 4
                    5 5 5
                    '
  [../]
[]
