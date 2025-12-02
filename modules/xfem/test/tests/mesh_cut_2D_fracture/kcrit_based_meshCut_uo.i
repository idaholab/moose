# both crack tips grow.  The right crack tip is arrested by the q-integral intersecting the boundary
[UserObjects]
  [cut_mesh2]
    type = MeshCut2DFractureUserObject
    mesh_generator_name = mesh_cutter
    growth_increment = 0.05
    ki_vectorpostprocessor = "II_KI_1"
    kii_vectorpostprocessor = "II_KII_1"
    k_critical = 100
  []
[]
