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
    order = CONSTANT
  []
  [u]
    family = MONOMIAL
    order = CONSTANT
  []
  [grad_u]
    family = L2_RAVIART_THOMAS
    # default first order means affine within the element interior but
    # the normal trace is constant along any given side. libMesh first
    # order corresponds to what is known in the literature as RT_0.
    # The lowest-order unstabilized HDG method here uses:
    # RT_0 for the vector field on the element interior
    # P_0 for the scalar field on the element interior
    # P_0 for the trace field on the mesh skeleton
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
