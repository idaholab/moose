!include child.i

[Postprocessors]
  [C_test]
    type = ConstantPostprocessor
    value = 1e-5
  []
  [C_rate_test]
    type = Receiver
  []
[]
