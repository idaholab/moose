#
# This is not a valid MOOSE input, it is only used for testing the HIT command
# line utility.
#

[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
  []
  [diff_v]
    type = Diffusion
    variable = v
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    value = 0
  []
  [right]
    type = NeumannBC
    variable = v
    value = 0
  []
[]

[PostProcessor]
  [min2]
    type = Calculation
    mode = MIN
    value = 0
  []
[]
