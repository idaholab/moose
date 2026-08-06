[Models]
  # q = 10 * jump + 1 * scale
  # 'jump' comes from a true InterfaceMaterial, 'scale' from a block material read on the
  # interface sides. Both feed the same NEML2 model to exercise mixed material-input sources.
  [mixed_cohesive]
    type = ScalarLinearCombination
    from = 'jump scale'
    to = 'q'
    weights = '10 1'
  []
[]
