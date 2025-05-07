[Models]
    [rom]
        type = LibtorchModel
        inputs = 'forces/T'
        outputs = 'state/k_T'
        file_path = 'solid_mechanics:libtorch/test/thermal_conductivity_model.pt'
        x_mean = '748.4263305664062'
        x_std = '432.9852600097656'
        y_mean = '66.79454040527344'
        y_std = '37.17350387573242'
    []
[]
