[GlobalParams]
  variable = face_vel_x
  u = vel_x
  v = vel_y
  grad_u = grad_vel_x
  grad_v = grad_vel_y
  face_u = face_vel_x
  face_v = face_vel_y
  pressure = p
  mu = 1
  rho = 0
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = -1
    ymax = 1
    nx = 10
    ny = 2
    elem_type = TRI6
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
    family = L2_LAGRANGE
  []
[]

[AuxVariables]
  [vel_x]
    family = L2_LAGRANGE
  []
  [vel_y]
    family = L2_LAGRANGE
  []
  [grad_vel_x]
    family = L2_LAGRANGE_VEC
  []
  [grad_vel_y]
    family = L2_LAGRANGE_VEC
  []
[]

[HDGKernels]
  [ns]
    type = NavierStokesHDGKernel
  []
[]

[HDGBCs]
  [walls]
    type = NavierStokesHDGVelocityDirichletBC
    boundary = 'bottom top'
  []
  [inlet]
    type = NavierStokesHDGVelocityDirichletBC
    boundary = 'left'
    dirichlet_u = '1'
  []
  [outlet]
    type = NavierStokesHDGOutflowBC
    boundary = 'right'
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
  csv = true
[]

[Postprocessors]
  [symmetric]
    type = IsMatrixSymmetric
  []
[]
