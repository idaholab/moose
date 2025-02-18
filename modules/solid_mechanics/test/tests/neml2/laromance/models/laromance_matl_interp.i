[Models]
  [rom_ep]
    type = LAROMANCE6DInterpolation
    model_file_name = 'models/random_value_6d_grid.json'
    model_file_variable_name = 'out_ep'
    output_rate = 'state/ep_rate'
    # grid nodes
    von_mises_stress = 'state/s'
    temperature = 'forces/T'
    equivalent_plastic_strain = 'state/ep'
    cell_dd_density = 'forces/cell_dd'
    wall_dd_density = 'forces/wall_dd'
    env_factor = 'forces/env_fac'
  []
  [rom_cell]
    type = LAROMANCE6DInterpolation
    model_file_name = 'models/random_value_6d_grid.json'
    model_file_variable_name = 'out_cell'
    output_rate = 'state/cell_rate'
    # grid nodes
    von_mises_stress = 'state/s'
    temperature = 'forces/T'
    equivalent_plastic_strain = 'state/ep'
    cell_dd_density = 'forces/cell_dd'
    wall_dd_density = 'forces/wall_dd'
    env_factor = 'forces/env_fac'
  []
  [rom_wall]
    type = LAROMANCE6DInterpolation
    model_file_name = 'models/random_value_6d_grid.json'
    model_file_variable_name = 'out_wall'
    output_rate = 'state/wall_rate'
    # grid nodes
    von_mises_stress = 'state/s'
    temperature = 'forces/T'
    equivalent_plastic_strain = 'state/ep'
    cell_dd_density = 'forces/cell_dd'
    wall_dd_density = 'forces/wall_dd'
    env_factor = 'forces/env_fac'
  []
  [combined_model]
    type = ComposedModel
    models = 'rom_ep rom_cell rom_wall'
  []
[]
