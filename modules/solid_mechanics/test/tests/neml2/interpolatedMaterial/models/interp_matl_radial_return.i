# HT9 Lafleur model input
# UNITS are MPa
[Solvers]
  [newton]
    type = Newton
    abs_tol = 1e-8
    rel_tol = 1e-6
    # verbose = true
  []
[]

[Models]
  #####################################################################################
  # Compute the invariant plastic flow direction since we are doing J2 radial return
  #####################################################################################
  [trial_elastic_strain]
    type = SR2LinearCombination
    to_var = 'state/Ee'
    from_var = 'forces/E old_state/Ep'
    coefficients = '1 -1'
  []
  [cauchy_stress]
    type = LinearIsotropicElasticity
    youngs_modulus = 170e3 #MPa
    poisson_ratio = 0.266
    strain = 'state/Ee'
    stress = 'state/S'
  []
  [flow_direction]
    type = J2FlowDirection
    mandel_stress = 'state/S'
    flow_direction = 'forces/N'
  []
  [trial_state]
    type = ComposedModel
    models = 'trial_elastic_strain cauchy_stress flow_direction'
  []

  #####################################################################################
  # Stress update
  #####################################################################################
  [ep_rate]
    type = ScalarVariableRate
    variable = 'state/ep'
    #  rate = 'state/ep_rate' #fixme do I need this
  []
  [plastic_strain_rate]
    type = AssociativePlasticFlow
    flow_direction = 'forces/N'
    flow_rate = 'state/ep_rate'
    plastic_strain_rate = 'state/Ep_rate'
  []
  [plastic_strain]
    type = SR2ForwardEulerTimeIntegration
    variable = 'state/Ep'
  []
  [plastic_update]
    type = ComposedModel
    models = 'ep_rate plastic_strain_rate plastic_strain'
  []
  [elastic_strain]
    type = SR2LinearCombination
    to_var = 'state/Ee'
    from_var = 'forces/E state/Ep'
    coefficients = '1 -1'
  []
  [stress_update]
    type = ComposedModel
    models = 'elastic_strain cauchy_stress'
  []

  #####################################################################################
  # Compute the rates of equivalent plastic strain and internal variables
  #####################################################################################
  [vonmises]
    type = SR2Invariant
    invariant_type = 'VONMISES'
    tensor = 'state/S'
    invariant = 'state/s'
  []
  [rom_ep]
    type = TabulatedEffectiveStrain
    model_file_name = 'models/random_value_grid.json'
    model_file_variable_name = 'out_ep'
    output_rate = 'state/ep_rate'
    # grid nodes
    von_mises_stress = 'state/s'
    equivalent_plastic_strain = 'state/ep'
    # fixme lynn sort out forces from old_forces from state.  wall & cell_dd should be old_forces.  env_fac never changes.
    cell_dd_density = 'old_state/cell_dd'
    wall_dd_density = 'old_state/wall_dd'
    temperature = 'forces/T'
    env_factor = 'forces/env_fac'
    # log10 transform variables
    factor = 0.0
    lowerbound = 0.0
    upperbound = 1.0
    logmin = -12 #-13.27790598614019
    logmax = 4 #3.8941036694177384
  []
  [integrate_ep]
    type = ScalarBackwardEulerTimeIntegration
    variable = 'state/ep'
  []
  [rate]
    type = ComposedModel
    models = "plastic_update stress_update vonmises rom_ep
              integrate_ep"
  []
  [radial_return]
    type = ImplicitUpdate
    implicit_model = 'rate'
    solver = 'newton'
  []
  #####################################################################################
  # Extra materials that evolve dislocation densities FIXME lynn I don't know how to evolve these
  #####################################################################################
  [rom_cell]
    type = TabulatedDislocationDensity
    model_file_name = 'models/random_value_grid.json'
    model_file_variable_name = 'out_cell'
    output_rate = 'state/cell_rate'
    # grid nodes
    von_mises_stress = 'state/s'
    equivalent_plastic_strain = 'state/ep'
    cell_dd_density = 'old_state/cell_dd'
    wall_dd_density = 'old_state/wall_dd'
    temperature = 'forces/T'
    env_factor = 'forces/env_fac'
    # compress transform variables
    factor = 1.0e-10
    compressor = 0.3
    original_min = -80 #-78.97871546935454
  []
  [rom_wall]
    type = TabulatedDislocationDensity
    model_file_name = 'models/random_value_grid.json'
    model_file_variable_name = 'out_wall'
    output_rate = 'state/wall_rate'
    # grid nodes
    von_mises_stress = 'state/s'
    equivalent_plastic_strain = 'state/ep'
    cell_dd_density = 'old_state/cell_dd'
    wall_dd_density = 'old_state/wall_dd'
    temperature = 'forces/T'
    env_factor = 'forces/env_fac'
    # compress transform variables
    factor = 1.0e-12
    compressor = 0.3
    original_min = -80 #-86.40245119107425
  []
  [cell_dd]
    type = ScalarForwardEulerTimeIntegration
    variable = 'state/cell_dd'
    rate = 'state/cell_rate'
  []
  [wall_dd]
    type = ScalarForwardEulerTimeIntegration
    variable = 'state/wall_dd'
    rate = 'state/wall_rate'
  []

  #####################################################################################
  # Put the models together (this is called by Bison)
  #####################################################################################
  [model]
    type = ComposedModel
    models = 'trial_state radial_return plastic_update stress_update vonmises rom_ep rom_cell rom_wall cell_dd wall_dd'
    priority = 'trial_state radial_return plastic_update stress_update vonmises rom_ep rom_cell rom_wall cell_dd wall_dd'
    additional_outputs = 'state/s state/ep state/S state/Ep state/ep_rate state/cell_rate state/wall_rate state/cell_dd state/wall_dd'
  []
[]
