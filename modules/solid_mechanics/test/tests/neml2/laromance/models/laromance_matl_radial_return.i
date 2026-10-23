# UNITS are MPa

[Models]
  #####################################################################################
  # Compute the invariant plastic flow direction since we are doing J2 radial return
  #####################################################################################
  [trial_elastic_strain]
    type = SR2LinearCombination
    from = 'neml2_strain plastic_strain~1'
    to = 'elastic_strain'
    weights = '1 -1'
  []
  [cauchy_stress]
    type = LinearIsotropicElasticity
    coefficient_types = 'YOUNGS_MODULUS POISSONS_RATIO' #MPa
    coefficients = '170e3 0.3'
    strain = 'elastic_strain'
    stress = 'neml2_stress'
  []
  [flow_direction]
    type = AssociativeJ2FlowDirection
    mandel_stress = 'neml2_stress'
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
    variable = 'equivalent_plastic_strain'
  []
  [plastic_strain_rate]
    type = AssociativePlasticFlow
    flow_rate = 'equivalent_plastic_strain_rate'
  []
  [plastic_strain]
    type = SR2ForwardEulerTimeIntegration
    variable = 'plastic_strain'
  []
  [plastic_update]
    type = ComposedModel
    models = 'ep_rate plastic_strain_rate plastic_strain'
  []
  [elastic_strain]
    type = SR2LinearCombination
    from = 'neml2_strain plastic_strain'
    to = 'elastic_strain'
    weights = '1 -1'
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
    tensor = 'neml2_stress'
    invariant = 'von_mises_stress'
  []
  [rom_ep]
    type = LAROMANCE6DInterpolation
    model_file_name = 'models/random_value_6d_grid.json'
    model_file_variable_name = 'out_ep'
    output_rate = 'equivalent_plastic_strain_rate'
    # Forward-Euler dislocation densities are explicit: evaluate the rate at the
    # old state so the densities stay inputs, not the integrators' new outputs.
    cell_dislocation_density = 'cell_dislocation_density~1'
    wall_dislocation_density = 'wall_dislocation_density~1'
  []
  [integrate_ep]
    type = ScalarBackwardEulerTimeIntegration
    variable = 'equivalent_plastic_strain'
  []
  [rate]
    type = ComposedModel
    models = 'plastic_update stress_update vonmises rom_ep integrate_ep'
  []
[]

[EquationSystems]
  [eq_sys]
    type = NonlinearSystem
    model = 'rate'
    unknowns = 'equivalent_plastic_strain'
  []
[]

[Solvers]
  [newton]
    type = Newton
    abs_tol = 1e-8
    rel_tol = 1e-6
    linear_solver = 'lu'
  []
  [lu]
    type = DenseLU
  []
[]

[Models]
  [predictor]
    type = ConstantExtrapolationPredictor
    unknowns_Scalar = 'equivalent_plastic_strain'
  []
  [radial_return]
    type = ImplicitUpdate
    equation_system = 'eq_sys'
    solver = 'newton'
    predictor = 'predictor'
  []
  #####################################################################################
  # Extra materials that evolve dislocation densities
  #####################################################################################
  [rom_cell]
    type = LAROMANCE6DInterpolation
    model_file_name = 'models/random_value_6d_grid.json'
    model_file_variable_name = 'out_cell'
    output_rate = 'cell_dislocation_density_rate'
    cell_dislocation_density = 'cell_dislocation_density~1'
    wall_dislocation_density = 'wall_dislocation_density~1'
  []
  [rom_wall]
    type = LAROMANCE6DInterpolation
    model_file_name = 'models/random_value_6d_grid.json'
    model_file_variable_name = 'out_wall'
    output_rate = 'wall_dislocation_density_rate'
    cell_dislocation_density = 'cell_dislocation_density~1'
    wall_dislocation_density = 'wall_dislocation_density~1'
  []
  [cell_dd]
    type = ScalarForwardEulerTimeIntegration
    variable = 'cell_dislocation_density'
  []
  [wall_dd]
    type = ScalarForwardEulerTimeIntegration
    variable = 'wall_dislocation_density'
  []

  #####################################################################################
  # Put the models together
  #####################################################################################
  [model]
    type = ComposedModel
    models = 'trial_state radial_return plastic_update stress_update vonmises rom_cell rom_wall cell_dd wall_dd'
    additional_outputs = 'von_mises_stress equivalent_plastic_strain neml2_stress plastic_strain cell_dislocation_density wall_dislocation_density'
  []
[]
