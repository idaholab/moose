[VectorPostprocessors]
  [CrackFrontNonlocalStressVpp]
    type = CrackFrontNonlocalStress
    stress_name = stress
    crack_front_definition = crackFrontDefinition
    box_length = 0.05
    box_height = 0.1
    execute_on = NONLINEAR
  []
[]
[UserObjects]
  [cut_mesh2]
    type = MeshCut2DFractureUserObject
    mesh_file = make_edge_crack_in.e
    growth_increment = 0.05
    stress_vectorpostprocessor = "CrackFrontNonlocalStressVpp"
    stress_vector_name = "crack_tip_stress"
    stress_threshold = 120
  []
[]
