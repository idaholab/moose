[Models]
  [model]
    type = NEML2TestModel
    # these are the default values for the parameters
    A = 'A'
    B = 'B'
    sum = 'sum'
    product = 'product'
  []
  [model_A]
    type = NEML2TestModel
    p1 = 2
    p2 = 3
  []
  [model_B]
    type = NEML2TestModel
    p1 = 4
    p2 = 5
  []
  [model_non_ad]
    type = NEML2TestModel
    ad = false
  []
[]
