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
  [model_A_non_ad]
    type = NEML2TestModel
    p1 = 2
    p2 = 3
    ad = false
  []
  [model_B_non_ad]
    type = NEML2TestModel
    p1 = 4
    p2 = 5
    ad = false
  []
  [error]
    type = NEML2TestModel
    error = true
    jit = false
  []
  [interface_source]
    type = NEML2TestModel2
  []
[]

[Schedulers]
  [simple]
    type = SimpleScheduler
    batch_size = 42
    device = 'cpu'
  []
[]
