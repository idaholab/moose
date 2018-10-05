#
# Difference objects with a cyclical dependency
#
[./diff1]
  type = Difference
  source1 = diff2
  source2 = diff2
[../]
[./diff2]
  type = Difference
  source1 = diff1
  source2 = diff1
[../]
