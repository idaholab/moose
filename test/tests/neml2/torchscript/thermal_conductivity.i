[Models]
  [kappa]
    type = LibtorchModel
    inputs = 'forces/T'
    outputs = 'state/k_T'
    file_path = 'libtorch/test/thermal_conductivity_model.pt'
  []
[]
