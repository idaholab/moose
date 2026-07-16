!include mms-channel.i

gamma = 1e4

[HDGKernels]
  [u_jump]
    type = MassFluxPenaltyIPHDG
    variable = vel_x
    face_variable = vel_bar_x
    face_velocity = face_velocity
    u = vel_x
    v = vel_y
    component = 0
    gamma = ${gamma}
  []
  [v_jump]
    type = MassFluxPenaltyIPHDG
    variable = vel_y
    face_variable = vel_bar_y
    face_velocity = face_velocity
    u = vel_x
    v = vel_y
    component = 1
    gamma = ${gamma}
  []
[]

[BCs]
  [u_jump_dirichlet]
    type = MassFluxPenaltyBC
    variable = vel_x
    face_variable = vel_bar_x
    face_velocity = dirichlet_velocity
    u = vel_x
    v = vel_y
    component = 0
    gamma = ${gamma}
    boundary = 'top bottom left'
    dirichlet_boundary = true
  []
  [v_jump_dirichlet]
    type = MassFluxPenaltyBC
    variable = vel_y
    face_variable = vel_bar_y
    face_velocity = dirichlet_velocity
    u = vel_x
    v = vel_y
    component = 1
    gamma = ${gamma}
    boundary = 'top bottom left'
    dirichlet_boundary = true
  []
  [u_jump_outlet]
    type = MassFluxPenaltyBC
    variable = vel_x
    face_variable = vel_bar_x
    face_velocity = face_velocity
    u = vel_x
    v = vel_y
    component = 0
    gamma = ${gamma}
    boundary = 'right'
    dirichlet_boundary = false
  []
  [v_jump_outlet]
    type = MassFluxPenaltyBC
    variable = vel_y
    face_variable = vel_bar_y
    face_velocity = face_velocity
    u = vel_x
    v = vel_y
    component = 1
    gamma = ${gamma}
    boundary = 'right'
    dirichlet_boundary = false
  []
[]

[FunctorMaterials]
  [face_velocity]
    type = ADGenericVectorFunctorMaterial
    prop_names = face_velocity
    prop_values = 'vel_bar_x vel_bar_y 0'
  []
  [dirichlet_velocity]
    type = GenericVectorFunctorMaterial
    prop_names = dirichlet_velocity
    prop_values = 'exact_u exact_v 0'
  []
[]
