[UserObjects]
  [cut_mesh2]
    type = MeshCut2DFractureUserObject
    mesh_file = make_edge_crack_in.e
    growth_increment = 0.05
    ki_vectorpostprocessor = "II_KI_1"
    kii_vectorpostprocessor = "II_KII_1"
    k_critical = 100
  []
[]
