!include ../3d-core.i

n = 16

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 0
    xmax = ${l}
    ymin = 0
    ymax = ${l}
    zmin = 0
    zmax = ${l}
    nx = ${n}
    ny = ${n}
    nz = ${n}
    elem_type = TET10
  []
[]

[BCs]
  [momentum_x_diffusion_walls]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = 'left bottom right back front'
    variable = vel_x
    face_variable = vel_bar_x
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = '0'
    diffusivity = 'mu'
    component = 0
  []
  [momentum_x_diffusion_top]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = 'top'
    variable = vel_x
    face_variable = vel_bar_x
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = '${U}'
    diffusivity = 'mu'
    component = 0
  []
  [momentum_y_diffusion_all]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = 'left bottom right top back front'
    variable = vel_y
    face_variable = vel_bar_y
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = '0'
    diffusivity = 'mu'
    component = 1
  []
  [momentum_z_diffusion_all]
    type = NavierStokesStressIPHDGDirichletBC
    boundary = 'left bottom right top back front'
    variable = vel_z
    face_variable = vel_bar_z
    pressure_variable = pressure
    pressure_face_variable = pressure_bar
    alpha = ${alpha}
    functor = '0'
    diffusivity = 'mu'
    component = 2
  []

  [pressure_walls]
    type = MassContinuityIPHDGBC
    face_variable = pressure_bar
    variable = pressure
    boundary = 'left bottom right back front'
    face_velocity_functors = '0 0 0'
    interior_velocity_vars = 'vel_x vel_y vel_z'
  []
  [pressure_lid]
    type = MassContinuityIPHDGBC
    face_variable = pressure_bar
    variable = pressure
    boundary = 'top'
    face_velocity_functors = '${U} 0 0'
    interior_velocity_vars = 'vel_x vel_y vel_z'
  []

  [pb_mass]
    type = MassMatrixIntegratedBC
    variable = pressure_bar
    matrix_tags = 'mass'
    boundary = 'left right bottom top back front'
    density = '${fparse -1/gamma}'
  []

  [u_jump_walls]
    type = MassFluxPenaltyBC
    variable = vel_x
    face_variable = vel_bar_x
    u = vel_x
    v = vel_y
    w = vel_z
    component = 0
    boundary = 'left right bottom back front'
    gamma = ${gamma}
    face_velocity = walls
    dirichlet_boundary = true
  []
  [v_jump_walls]
    type = MassFluxPenaltyBC
    variable = vel_y
    face_variable = vel_bar_y
    u = vel_x
    v = vel_y
    w = vel_z
    component = 1
    boundary = 'left right bottom back front'
    gamma = ${gamma}
    face_velocity = walls
    dirichlet_boundary = true
  []
  [w_jump_walls]
    type = MassFluxPenaltyBC
    variable = vel_z
    face_variable = vel_bar_z
    u = vel_x
    v = vel_y
    w = vel_z
    component = 2
    boundary = 'left right bottom back front'
    gamma = ${gamma}
    face_velocity = walls
    dirichlet_boundary = true
  []
  [u_jump_top]
    type = MassFluxPenaltyBC
    variable = vel_x
    face_variable = vel_bar_x
    u = vel_x
    v = vel_y
    w = vel_z
    component = 0
    boundary = 'top'
    gamma = ${gamma}
    face_velocity = top_vel
    dirichlet_boundary = true
  []
  [v_jump_top]
    type = MassFluxPenaltyBC
    variable = vel_y
    face_variable = vel_bar_y
    u = vel_x
    v = vel_y
    w = vel_z
    component = 1
    boundary = 'top'
    gamma = ${gamma}
    face_velocity = top_vel
    dirichlet_boundary = true
  []
  [w_jump_top]
    type = MassFluxPenaltyBC
    variable = vel_z
    face_variable = vel_bar_z
    u = vel_x
    v = vel_y
    w = vel_z
    component = 2
    boundary = 'top'
    gamma = ${gamma}
    face_velocity = top_vel
    dirichlet_boundary = true
  []
[]

[FunctorMaterials]
  [top]
    type = GenericVectorFunctorMaterial
    prop_names = top_vel
    prop_values = '${U} 0 0'
  []
  [walls]
    type = GenericVectorFunctorMaterial
    prop_names = walls
    prop_values = '0 0 0'
  []
[]
