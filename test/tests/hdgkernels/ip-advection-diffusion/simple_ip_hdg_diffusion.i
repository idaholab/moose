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
  []
[]

[Postprocessors]
  [symmetric]
    type = IsMatrixSymmetric
  []
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-10
  solve_type = NEWTON
  # the backtracking line search requires an explicit matrix
  line_search = 'cp'
[]

[Outputs]
  exodus = true
  [dof]
    type = DOFMap
    execute_on = 'initial'
  []
[]
