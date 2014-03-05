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

  strips = 3  # this is a required params (miss-spelled) => expect to error out
[]

[Functions]
  [./forcing_fn]
    type = ParsedFunction
    value = x
  [../
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
    type = UserForcingFunction
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
   exodus = true
  [./console]
    type = Console
    perf_log = true
  [../]
[]
