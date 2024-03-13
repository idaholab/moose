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
    nx = 2
    ny = 2
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

[HDGKernels]
  [diff]
    type = DiffusionHDGKernel
    source = 'forcing'
  []
[]

[HDGBCs]
  [all]
    type = DiffusionHDGDirichletBC
    boundary = 'left right top bottom'
    functor = 'exact'
  []
[]

[Functions]
  [exact]
    type = ParsedFunction
    expression = 'cos(.5*pi*x)*sin(.5*pi*y)'
  []
  [forcing]
    type = ParsedFunction
    expression = '.5*pi*.5*pi*cos(.5*pi*x)*sin(.5*pi*y) + .5*pi*.5*pi*cos(.5*pi*x)*sin(.5*pi*y)'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre    boomeramg'
  line_search = 'basic'
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [L2u]
    type = ElementL2Error
    variable = u
    function = exact
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]
