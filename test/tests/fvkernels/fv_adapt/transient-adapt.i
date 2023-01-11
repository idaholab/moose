[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    elem_type = QUAD4
  []
[]

[Variables]
  [u]
    order = CONSTANT
    family = MONOMIAL
  []
  [v][]
[]

[Functions]
  [force]
    type = ParsedFunction
    expression = t
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = v
  []
  [force]
    type = BodyForce
    variable = v
    function = force
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = u
    coeff = coeff
  []
  [force]
    type = FVBodyForce
    variable = u
    function = force
  []
[]

[FVBCs]
  [right]
    type = FVDirichletBC
    variable = u
    boundary = right
    value = 1
  []
  [left]
    type = FVDirichletBC
    variable = u
    boundary = left
    value = 0
  []
[]

[BCs]
  [right]
    type = DirichletBC
    variable = v
    boundary = right
    value = 1
  []
  [left]
    type = DirichletBC
    variable = v
    boundary = left
    value = 0
  []
[]

[Materials]
  [diff]
    type = ADGenericFunctorMaterial
    prop_names = 'coeff'
    prop_values = '1'
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 1
  solve_type = 'NEWTON'
[]

[Adaptivity]
  marker = box
  initial_steps = 1
  [Markers]
    [box]
      bottom_left = '0.3 0.3 0'
      inside = refine
      top_right = '0.6 0.6 0'
      outside = do_nothing
      type = BoxMarker
    []
  []
[]

[Outputs]
  exodus = true
[]
