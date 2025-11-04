[GlobalParams]
  variable = u
  gradient_variable = grad_u
  face_variable = face_u
  diffusivity = 1
  tau = 0
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
  [u]
    family = MONOMIAL
    order = CONSTANT
  []
  [grad_u]
    family = L2_RAVIART_THOMAS
  []
[]

[HDGKernels]
  [diff]
    type = DiffusionLHDGKernel
    source = 'forcing'
  []
[]

[BCs]
  [all]
    type = DiffusionLHDGDirichletBC
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

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  line_search = 'basic'
[]

[Outputs]
  print_linear_residuals = false
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
