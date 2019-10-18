[Mesh]
  [./rmg]
    type = RinglebMeshGenerator
    kmin = 0.7
    num_k_pts = 9
    num_q_pts = 20
    kmax = 1.2
    n_extra_q_pts = 2
    gamma = 1.4
    triangles = true
  []
[]

[Outputs]
  exodus = true
[]
