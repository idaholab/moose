[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0.01 # to make sure the meshes don't align
    xmax = 0.49 # to make sure the meshes don't align
    ymax = 1
    nx = 10
    ny = 10
  []
  [block1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '0.2 0.2 0'
    top_right = '0.3 0.8 0'
  []
[]

[Variables]
  [sink]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Functions]
  [sink_func]
    type = ParsedFunction
    expression = '5e2*x*(0.5-x)+5e1'
  []
[]

[Kernels]
  [reaction]
    type = Reaction
    variable = sink
  []

  [coupledforce]
    type = BodyForce
    variable = sink
    function = sink_func
  []
[]

[AuxVariables]
  [from_parent]
    block = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Postprocessors]
  [sink]
    type = ElementIntegralVariablePostprocessor
    block = 1
    variable = sink
  []
  [from_parent_pp]
    type = ElementIntegralVariablePostprocessor
    block = 1
    variable = from_parent
    execute_on = 'transfer'
  []
[]

[Outputs]
  exodus = true
  [console]
    type = Console
    execute_on = 'timestep_end timestep_begin'
  []
[]
