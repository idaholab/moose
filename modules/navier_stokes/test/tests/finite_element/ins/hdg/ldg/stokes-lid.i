[GlobalParams]
  u = vel_x
  v = vel_y
  grad_u = grad_vel_x
  grad_v = grad_vel_y
  face_u = face_vel_x
  face_v = face_vel_y
  pressure = p
  enclosure_lm = lm
  mu = 1
  rho = 0
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -1
    xmax = 1
    ymin = -1
    ymax = 1
    nx = 4
    ny = 4
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
  [lm]
    family = SCALAR
  []
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
    type = NavierStokesLHDGKernel
  []
[]

[BCs]
  [walls]
    type = NavierStokesLHDGVelocityDirichletBC
    boundary = 'left right bottom'
    dirichlet_u = 0
    dirichlet_v = 0
  []
  [lid]
    type = NavierStokesLHDGVelocityDirichletBC
    boundary = 'top'
    dirichlet_u = 1
    dirichlet_v = 0
  []
[]

[Preconditioning]
  [sc]
    type = StaticCondensation
    petsc_options_iname = '-pc_type -pc_factor_shift_type -ksp_view_pmat'
    petsc_options_value = 'lu       NONZERO               binary'
    dont_condense_vars = 'p'
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
[]

[Outputs]
  hide = 'lm'
  csv = true
[]

[Postprocessors]
  [symmetric]
    type = MatrixSymmetryCheck
    mat = binaryoutput
  []
[]
