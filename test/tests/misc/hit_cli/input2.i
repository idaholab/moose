#
# This is not a valid MOOSE input, it is only used for testing the HIT command
# line utility.
#

[Outputs]
  [out]
    type = Exodus
  []
[]

[PostProcessor]
  [max]
    type = Calculation
    mode = MAX
    value = 1
  []
  [min]
    type = Calculation
    mode = min
    value = 1
  []
  [min2]
    type = Calculation
    mode = MIN
    value = 1
  []
[]
