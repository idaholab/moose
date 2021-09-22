[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmin = 0
  ymin = 0
  uniform_refine = 2
[]

[AuxVariables]
  [variable]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 2
  []
[]

[Postprocessors]
  [variable]
    type = FunctionValuePostprocessor
    function = 3
    execute_on = 'initial'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs/out]
  type = Exodus
  elemental_as_nodal = true
[]
