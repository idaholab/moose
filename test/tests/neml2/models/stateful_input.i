# NEML2TestModel with its input 'A' renamed to 'foo~1' -- i.e. the old (stateful) value of a
# variable 'foo' that the model never produces: 'foo' is neither a current model input nor a model
# output. MOOSE therefore has no material property to source the old value from, which is exactly
# the stateful misconfiguration NEML2Action::checkStatefulConsistency (guard 2) reports instead of
# crashing during stateful material property initialization.
[Models]
  [model]
    type = NEML2TestModel
    A = 'foo~1'
    B = 'B'
    sum = 'sum'
    product = 'product'
  []
[]
