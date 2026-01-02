
[Mesh]
  partitioner = 'linear'
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    xmax = 2
    ymax = 2
    elem_type = QUAD9
  []
[]

[Variables]
  [u]
    family = HIERARCHIC
    order = second
  []
  [v]
    family = HIERARCHIC
    order = second
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [body]
    type = BodyForce
    variable = u
    function = forcing_u
  []
  [diff_v]
    type = Diffusion
    variable = v
  []
  [body_v]
    type = BodyForce
    variable = v
    function = forcing_v
  []
[]

[BCs]
  [all]
    type = FunctionDirichletBC
    boundary = 'left right top bottom'
    function = 'exact_u'
    variable = u
  []
  [all_v]
    type = FunctionDirichletBC
    boundary = 'left right top bottom'
    function = 'exact_v'
    variable = v
  []
[]

[Functions]
  [exact_u]
    type = ParsedFunction
    expression = 'x^(1/2)'
  []
  [forcing_u]
    type = ParsedFunction
    expression = '(1/4)/x^(3/2)'
  []
  [exact_v]
    type = ParsedFunction
    expression = '(x + 1)^(1/2)'
  []
  [forcing_v]
    type = ParsedFunction
    expression = '(1/4)/(x + 1)^(3/2)'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Adaptivity]
  adaptivity_type = HP
  marker = 'combo'
  steps = 3
  max_h_level = 3
  [Markers]
    [combo]
      type = ComboMarker
      markers = 'jump_u jump_v'
    []
    [jump_u]
      type = ErrorFractionMarker
      coarsen = 0.05
      refine = 0.7
      indicator = GJI_u
    []
    [jump_v]
      type = ErrorFractionMarker
      coarsen = 0.05
      refine = 0.7
      indicator = GJI_v
    []
  []
  [Indicators]
    [GJI_u]
      type = GradientJumpIndicator
      variable = u
    []
    [GJI_v]
      type = GradientJumpIndicator
      variable = v
    []
  []
[]

[AuxVariables]
  [p_level]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [elem_p_level]
    type = ElementAdaptivityLevelAux
    level = p
    variable = p_level
  []
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  [num_dofs_nl]
    type = NumDOFs
    system = 'nl'
  []
  [num_dofs_aux]
    type = NumDOFs
    system = 'aux'
  []
  [num_elems]
    type = NumElements
  []
  [avg_u]
    type = ElementAverageValue
    variable = u
  []
  [avg_v]
    type = ElementAverageValue
    variable = v
  []
  [avg_p_level]
    type = ElementAverageValue
    variable = p_level
  []
[]
