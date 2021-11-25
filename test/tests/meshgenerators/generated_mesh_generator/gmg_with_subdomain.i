[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 3
    ny = 3
    nz = 4
    bias_x = 2
    bias_z = 0.5
    subdomain_ids = '0 1 2
                     0 0 1
                     1 2 2

                     0 1 2
                     0 0 1
                     1 2 2

                     0 1 2
                     0 0 1
                     1 2 2

                     0 1 1
                     0 1 1
                     1 2 2
                    '
  []
[]

[Outputs]
  exodus = true
[]
