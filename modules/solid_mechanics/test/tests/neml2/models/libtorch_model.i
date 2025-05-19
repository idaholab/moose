[Models]
    [rom]
        type = LibtorchModel
        inputs = 'forces/T'
        outputs = 'state/k_T'
        file_path = 'solid_mechanics:libtorch/test/thermal_conductivity_model.pt'
    []
[]
