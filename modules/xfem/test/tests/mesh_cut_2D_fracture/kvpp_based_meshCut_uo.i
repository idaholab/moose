[VectorPostprocessors]
  [CrackFrontNonlocalScalarVpp]
    type = CrackFrontNonlocalScalarMaterial
    property_name = k_crit_mat
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
    ki_vectorpostprocessor = "II_KI_1"
    kii_vectorpostprocessor = "II_KII_1"
    k_critical_vectorpostprocessor=CrackFrontNonlocalScalarVpp
    k_critical_vector_name = "crack_tip_k_crit_mat"
  []
[]
[Materials]
  [k_critical]
    type = ParsedMaterial
    property_name = k_crit_mat
    extra_symbols = 'x'
    expression = 'if(x < -0.5,500,if(x<0.3,40,500))'
  []
[]
