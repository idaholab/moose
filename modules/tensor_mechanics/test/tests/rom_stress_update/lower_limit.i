temp = 800.0160634

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
  [./temperature]
    initial_condition = ${temp}
  [../]
[]

[Functions]
  [./strain_exact]
    type = ParsedFunction
    vars = 'lower_limit_strain lower_limit_temperature temp_avg'
    vals = '2.842181e-09       800.0160634             temp_avg'
    value = 'val := temp_avg / lower_limit_temperature;
             clamped := if(val < 0, 0, if(val > 1, 1, val));
             smootherstep := clamped^3 * (clamped * (clamped * 6.0 - 15.0) + 10.0);
             smootherstep * lower_limit_strain'
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    add_variables = true
    use_automatic_differentiation = true
    generate_output = vonmises_stress
  [../]
[]

[BCs]
  [./symmy]
    type = ADDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  [../]
  [./symmx]
    type = ADDirichletBC
    variable = disp_x
    boundary = left
    value = 0
  [../]
  [./symmz]
    type = ADDirichletBC
    variable = disp_z
    boundary = back
    value = 0
  [../]
  [./pressure_x]
    type = ADPressure
    variable = disp_x
    component = 0
    boundary = right
    constant = 1.0e5
  [../]
  [./pressure_y]
    type = ADPressure
    variable = disp_y
    component = 1
    boundary = top
    constant = -1.0e5
  [../]
  [./pressure_z]
    type = ADPressure
    variable = disp_z
    component = 2
    boundary = front
    constant = -1.0e5
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 3.30e11
    poissons_ratio = 0.3
  [../]
  [./stress]
    type = ADComputeMultipleInelasticStress
    inelastic_models = rom_stress_prediction
  [../]
  [./rom_stress_prediction]
    type = SS316HLAROMANCEStressUpdateTest
    temperature = temperature
    initial_mobile_dislocation_density = 6.0e12
    initial_immobile_dislocation_density = 4.4e11
    outputs = all
  [../]
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
  [./strain_exact]
    type = FunctionValuePostprocessor
    function = strain_exact
  [../]
  [./strain_avg]
    type = ElementAverageValue
    variable = effective_creep_strain
  [../]
  [./strain_diff]
    type = DifferencePostprocessor
    value1 = strain_exact
    value2 = strain_avg
  [../]
  [./temp_avg]
    type = ElementAverageValue
    variable = temperature
  [../]
  [./mobile_dislocations]
    type = ElementAverageValue
    variable = mobile_dislocations
  [../]
  [./immobile_disloactions]
    type = ElementAverageValue
    variable = immobile_dislocations
  [../]
  [./vonmises_stress]
    type = ElementAverageValue
    variable = vonmises_stress
  [../]
[]

[Outputs]
  csv = true
[]
