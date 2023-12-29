temp = 800.0160634
disp = 1.0053264195e6

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[AuxVariables]
  [temperature]
    initial_condition = ${temp}
  []
[]

[Functions]
  [temp_weight]
    type = ParsedFunction
    symbol_names = 'lower_limit avg'
    symbol_values = '800.0160634 temp_avg'
    expression = 'val := 2 * avg / lower_limit - 1;
             clamped := if(val <= -1, -0.99999, if(val >= 1, 0.99999, val));
             plus := exp(-2 / (1 + clamped));
             minus := exp(-2 / (1 - clamped));
             plus / (plus + minus)'
  []
  [stress_weight]
    type = ParsedFunction
    symbol_names = 'lower_limit avg'
    symbol_values = '2.010652839e6 vonmises_stress'
    expression = 'val := 2 * avg / lower_limit - 1;
             clamped := if(val <= -1, -0.99999, if(val >= 1, 0.99999, val));
             plus := exp(-2 / (1 + clamped));
             minus := exp(-2 / (1 - clamped));
             plus / (plus + minus)'
  []
  [creep_rate_exact]
    type = ParsedFunction
    symbol_names = 'lower_limit_strain temp_weight stress_weight'
    symbol_values = '3.370764e-12       temp_weight stress_weight'
    expression = 'lower_limit_strain * temp_weight * stress_weight'
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = FINITE
    add_variables = true
    generate_output = vonmises_stress
  []
[]

[BCs]
  [symmy]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [symmx]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [symmz]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0
  []
  [pressure_x]
    type = Pressure
    variable = disp_x
    boundary = right
    factor = ${disp}
  []
  [pressure_y]
    type = Pressure
    variable = disp_y
    boundary = top
    factor = -${disp}
  []
  [pressure_z]
    type = Pressure
    variable = disp_z
    boundary = front
    factor = -${disp}
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 3.30e11
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = rom_stress_prediction
  []
  [rom_stress_prediction]
    type = SS316HLAROMANCEStressUpdateTest
    temperature = temperature
    initial_cell_dislocation_density = 6.0e12
    initial_wall_dislocation_density = 4.4e11
    outputs = all
    apply_strain = false
  []
[]

[Executioner]
  type = Transient

  solve_type = 'NEWTON'

  nl_abs_tol = 1e-12
  automatic_scaling = true
  compute_scaling_once = false

  num_steps = 1
  dt = 1e5
[]

[Postprocessors]
  [creep_rate_exact]
    type = FunctionValuePostprocessor
    function = creep_rate_exact
  []
  [creep_rate_avg]
    type = ElementAverageValue
    variable = creep_rate
  []
  [creep_rate_diff]
    type = DifferencePostprocessor
    value1 = creep_rate_exact
    value2 = creep_rate_avg
  []
  [temp_avg]
    type = ElementAverageValue
    variable = temperature
  []
  [cell_dislocations]
    type = ElementAverageValue
    variable = cell_dislocations
  []
  [wall_disloactions]
    type = ElementAverageValue
    variable = wall_dislocations
  []
  [vonmises_stress]
    type = ElementAverageValue
    variable = vonmises_stress
  []
[]

[Outputs]
  csv = true
[]
