[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  second_order = true
[]

[Variables]
  [v]
    order = SECOND
  []
  [u]
    order = FIRST
    family = MONOMIAL
  []
  [side_u]
    order = FIRST
    family = SIDE_HIERARCHIC
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = v
  []
[]

[HDGKernels]
  [diff]
    type = DiffusionIPHDGKernel
    variable = u
    face_variable = side_u
    diffusivity = 1
    alpha = 6
  []
[]

[BCs]
  [left]
    type = DiffusionIPHDGDirichletBC
    functor = 0
    boundary = 'left'
    variable = u
    face_variable = side_u
    diffusivity = 1
    alpha = 6
  []
  [right]
    type = DiffusionIPHDGDirichletBC
    functor = 1
    boundary = 'right'
    variable = u
    face_variable = side_u
    diffusivity = 1
    alpha = 6
  []
  [zero_flux]
    type = DiffusionIPHDGPrescribedFluxBC
    boundary = 'top bottom'
    variable = u
    face_variable = side_u
    diffusivity = 1
    alpha = 6
    prescribed_normal_flux = 0
  []
  [left_v]
    type = PenaltyDirichletBC
    variable = v
    boundary = left
    value = 0
    penalty = 1e8
  []
  [right_v]
    type = PenaltyDirichletBC
    variable = v
    boundary = right
    value = 1
    penalty = 1e8
  []
[]

[Preconditioning]
  [sc]
    type = StaticCondensation
    petsc_options = '-ksp_view'
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'hypre'
    dont_condense_vars = 'u'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
  hide = 'side_u'
[]
