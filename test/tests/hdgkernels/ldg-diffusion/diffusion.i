[GlobalParams]
  u = u
  grad_u = grad_u
  face_u = face_u
  diffusivity = 1
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    elem_type = TRI6
  []
[]

[Variables]
  [face_u]
    family = SIDE_HIERARCHIC
  []
  [u]
    family = L2_LAGRANGE
  []
  [grad_u]
    family = L2_LAGRANGE_VEC
  []
[]

[HDGKernels]
  [diff]
    type = DiffusionLHDGKernel
  []
[]

[BCs]
  inactive = 'right_flux'
  [left]
    type = DiffusionLHDGDirichletBC
    boundary = 'left'
    functor = '0'
  []
  [right]
    type = DiffusionLHDGDirichletBC
    boundary = 'right'
    functor = '1'
  []
  [right_flux]
    type = DiffusionLHDGPrescribedGradientBC
    boundary = 'right'
    normal_gradient = 1
  []
  [zero_flux]
    type = DiffusionLHDGPrescribedGradientBC
    boundary = 'top bottom'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  # the backtracking line search requires an explicit matrix
  line_search = 'cp'
[]

[Postprocessors]
  [left]
    type = SideAverageValue
    boundary = left
    variable = u
  []
  [right]
    type = SideAverageValue
    boundary = right
    variable = u
  []
  [bottom]
    type = SideAverageValue
    boundary = bottom
    variable = u
  []
  [top]
    type = SideAverageValue
    boundary = top
    variable = u
  []
  [symmetric]
    type = IsMatrixSymmetric
  []
[]

[Outputs]
  csv = true
[]
