!include physics.i
!include off_diag_coupling.i
!include nlp.i

[Kernels]
  [coupled_uv]
    matrix_tags = 'NPC_J_0_1'
  []
  [coupled_vu]
    matrix_tags = 'NPC_J_1_0'
  []
[]

[BCs]
  [left_u]
    extra_matrix_tags = 'NPC_J_0_1'
  []
  [right_u]
    extra_matrix_tags = 'NPC_J_0_1'
  []
  [left_v]
    extra_matrix_tags = 'NPC_J_1_0'
  []
  [right_v]
    extra_matrix_tags = 'NPC_J_1_0'
  []
[]
