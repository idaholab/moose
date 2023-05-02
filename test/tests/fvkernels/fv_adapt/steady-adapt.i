[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 1
    elem_type = QUAD4
  []
[]

[Variables]
  [u]
    order = CONSTANT
    family = MONOMIAL
    fv = true
    type = MooseVariableFVReal
  []
[]

[Functions]
  [exact-quadratic]
    type = ParsedFunction
    expression = '-(x-1)^2+1'
  []
  [exact-linear]
    type = ParsedFunction
    expression = 'x'
  []
[]

[FVKernels]
  inactive = 'source'
  [diff]
    type = FVDiffusion
    variable = u
    coeff = coeff
    use_point_neighbors = true
  []
  [source]
    type = FVBodyForce
    variable = u
    function = 2
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

[Materials]
  [diff]
    type = ADGenericFunctorMaterial
    prop_names = 'coeff'
    prop_values = '1'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'hypre'
[]

[Adaptivity]
  marker = box
  initial_steps = 1
  [Markers]
    [box]
      bottom_left = '0.5 0 0'
      inside = refine
      top_right = '1 1 0'
      outside = do_nothing
      type = BoxMarker
    []
  []
[]

[Outputs]
  exodus = true
  csv = true
  [console]
    type = Console
    system_info = 'framework mesh aux nonlinear relationship execution'
  []
[]

[Postprocessors]
  [error]
    type = ElementL2Error
    variable = u
    function = exact-linear
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
  [h]
    type = AverageElementSize
    outputs = 'console csv'
    execute_on = 'timestep_end'
  []
[]
