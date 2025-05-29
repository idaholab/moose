[GlobalParams]
  variable = u
  face_variable = side_u
  diffusivity = 1
  alpha = 6
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  elem_type = QUAD9
[]

[Variables]
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
  [time]
    type = TimeDerivative
    variable = u
  []
[]

[HDGKernels]
  [diff]
    type = DiffusionIPHDGKernel
  []
[]

[BCs]
  [left]
    type = DiffusionIPHDGDirichletBC
    functor = 0
    boundary = 'left'
  []
  [right]
    type = DiffusionIPHDGDirichletBC
    functor = 1
    boundary = 'right'
  []
  [zero_flux]
    type = DiffusionIPHDGPrescribedFluxBC
    boundary = 'top bottom'
    prescribed_normal_flux = 0
  []
[]

[Preconditioning]
  [smp]
    type = StaticCondensation
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 0.2
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
  solve_type = PJFNK
  petsc_options_iname = '-ksp_type'
  petsc_options_value = 'preonly'
  line_search = 'none'
[]

[Outputs]
  exodus = true
  hide = 'side_u'
[]
