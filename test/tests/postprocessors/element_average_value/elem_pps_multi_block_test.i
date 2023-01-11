#
# Tests elemental PPS running on multiple block
#
[Mesh]
  type = StripeMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 3
  ny = 3
  elem_type = QUAD4
  stripes = 3
  # StripeMesh currently only works correctly with ReplicatedMesh.
  parallel_type = replicated
[]

[Functions]
  [./forcing_fn]
    type = ParsedFunction
    expression = x
  [../]
[]

[Variables]
  [./u]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[Kernels]
  [./uv]
    type = Reaction
    variable = u
  [../]

  [./fv]
    type = BodyForce
    variable = u
    function = forcing_fn
  [../]
[]

[Postprocessors]
  [./avg_1_2]
    type = ElementAverageValue
    variable = u
    block = '0 1'
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = 'timestep_end'
  [./out]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]
