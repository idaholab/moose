[Models]
  [rom_ep]
    type = TabulatedEffectiveStrain
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
    # log10 transform variables
    factor = 0.0
    lowerbound = 0.0
    upperbound = 1.0
    logmin = -12
    logmax = 4
  []
  [rom_cell]
    type = TabulatedDislocationDensity
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
    # compress transform variables
    factor = 1.0e-10
    compressor = 0.3
    original_min = -80
  []
  [rom_wall]
    type = TabulatedDislocationDensity
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
    # compress transform variables
    factor = 1.0e-12
    compressor = 0.3
    original_min = -80
  []
  [combined_model]
    type = ComposedModel
    models = 'rom_ep rom_cell rom_wall'
  []
[]
