U=1
l=1
n=4
nu=1

[GlobalParams]
  variable = face_vel_x
  u = vel_x
  v = vel_y
  grad_u = grad_vel_x
  grad_v = grad_vel_y
  face_u = face_vel_x
  face_v = face_vel_y
  pressure = p
  enclosure_lm = lm
  nu = ${nu}
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${l}
    ymin = 0
    ymax = ${l}
    nx = ${n}
    ny = ${n}
    elem_type = QUAD9
  []
[]

[Variables]
  [face_vel_x]
    family = SIDE_HIERARCHIC
  []
  [face_vel_y]
    family = SIDE_HIERARCHIC
  []
  [p]
    family = L2_HIERARCHIC
  []
  [lm]
    family = SCALAR
  []
[]

[AuxVariables]
  [vel_x]
    family = L2_HIERARCHIC
  []
  [vel_y]
    family = L2_HIERARCHIC
  []
  [grad_vel_x]
    family = L2_HIERARCHIC_VEC
  []
  [grad_vel_y]
    family = L2_HIERARCHIC_VEC
  []
[]

[HybridizedKernels]
  [ns]
    type = NavierStokesHybridizedKernel
  []
[]

[HybridizedBCs]
  [walls]
    type = NavierStokesHybridizedVelocityDirichletBC
    boundary = 'left right bottom'
  []
  [lid]
    type = NavierStokesHybridizedVelocityDirichletBC
    boundary = 'top'
    dirichlet_u = '${U}'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  line_search = 'basic'
[]

[Outputs]
  exodus = true
[]
