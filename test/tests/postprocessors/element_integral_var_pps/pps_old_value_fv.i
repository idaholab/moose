[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 4
  ny = 4
  elem_type = QUAD4
[]

[Variables]
  [./u]
    order = CONSTANT
    family = MONOMIAL
    fv = true
    initial_condition = 1
  [../]
[]

[Functions]
  [./force_fn]
    type = ParsedFunction
    expression = '1'
  [../]

  [./exact_fn]
    type = ParsedFunction
    expression = 't'
  [../]
[]

[FVKernels]
  [./diff_u]
    type = FVDiffusion
    variable = u
    coeff = '1'
    block = '0'
  [../]

  [./ffn_u]
    type = FVBodyForce
    variable = u
    function = force_fn
  [../]
[]

[FVBCs]
  [./all_u]
    type = FVFunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
  [../]
[]

[Postprocessors]
  [./a]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = 'initial timestep_end'
  [../]

  [./total_a]
    type = TimeIntegratedPostprocessor
    value = a
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
