[Models]
  [model]
    type = NEML2TestModel
    # these are the default values for the parameters
    A = forces/A
    B = forces/B
    sum = state/internal/sum
    product = state/internal/product
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

[Schedulers]
  [simple]
    type = SimpleScheduler
    batch_size = 42
    device = 'cpu'
  []
[]
