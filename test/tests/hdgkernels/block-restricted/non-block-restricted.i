[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 1
    elem_type = QUAD9
    xmax = 2
  []
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
  [left_u]
    type = DiffusionIPHDGDirichletBC
    functor = 0
    boundary = 'left'
    alpha = 6
    variable = u
    face_variable = side_u
    diffusivity = 1
  []
  [zero_flux_u]
    type = DiffusionIPHDGPrescribedFluxBC
    boundary = 'top bottom'
    prescribed_normal_flux = 0
    variable = u
    face_variable = side_u
    alpha = 6
    diffusivity = 1
  []
  [right_u]
    type = DiffusionIPHDGDirichletBC
    functor = 1
    boundary = 'right'
    alpha = 6
    variable = u
    face_variable = side_u
    diffusivity = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       mumps'
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  [avg_u]
    type = ElementAverageValue
    variable = u
  []
  [avg_side_u]
    type = ElementAverageValue
    variable = side_u
  []
[]
