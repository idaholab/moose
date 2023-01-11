#
# Tests elemental PPS running on multiple blocks
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
    expression = x*(y+1)
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
  [./int_0_1]
    type = SideIntegralVariablePostprocessor
    variable = u
    boundary = '0 1'
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
