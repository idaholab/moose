[Models]
  [rom_ep]
    type = LAROMANCE6DInterpolation
    model_file_name = 'models/random_value_6d_grid.json'
    model_file_variable_name = 'out_ep'
    output_rate = 'equivalent_plastic_strain_rate'
  []
  [rom_cell]
    type = LAROMANCE6DInterpolation
    model_file_name = 'models/random_value_6d_grid.json'
    model_file_variable_name = 'out_cell'
    output_rate = 'cell_dislocation_density_rate'
  []
  [rom_wall]
    type = LAROMANCE6DInterpolation
    model_file_name = 'models/random_value_6d_grid.json'
    model_file_variable_name = 'out_wall'
    output_rate = 'wall_dislocation_density_rate'
  []
  [combined_model]
    type = ComposedModel
    models = 'rom_ep rom_cell rom_wall'
  []
[]
