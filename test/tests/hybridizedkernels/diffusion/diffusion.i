[GlobalParams]
  variable = face_u
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
[]

[AuxVariables]
  [u]
    family = L2_LAGRANGE
  []
  [grad_u]
    family = L2_LAGRANGE_VEC
  []
[]

[HybridizedKernels]
  [diff]
    type = DiffusionHybridizedKernel
  []
[]

[HybridizedBCs]
  [left]
    type = DiffusionHybridizedDirichletBC
    boundary = 'left'
    function = '0'
  []
  [right]
    type = DiffusionHybridizedDirichletBC
    boundary = 'right'
    function = '1'
  []
  [zero_flux]
    type = DiffusionHybridizedZeroFluxBC
    boundary = 'top bottom'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  line_search = 'basic'
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
[]

[Outputs]
  csv = true
[]
