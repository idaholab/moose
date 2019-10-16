[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Problem]
  coord_type = RZ
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[AuxVariables]
  [./temperature]
    initial_condition = 900.0
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    add_variables = true
    use_automatic_differentiation = true
  [../]
[]

[BCs]
  [./symmy]
    type = ADPresetBC
    variable = disp_y
    boundary = bottom
    value = 0
  [../]
  [./symmx]
    type = ADPresetBC
    variable = disp_x
    boundary = left
    value = 0
  [../]
  [./pressure_x]
    type = ADPressure
    variable = disp_x
    component = 0
    boundary = right
    constant = 3.1675e5
  [../]
  [./pressure_y]
    type = ADPressure
    variable = disp_y
    component = 1
    boundary = top
    constant = 6.336e5
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
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

  num_steps = 5
[]

[Postprocessors]
  [./effective_strain_avg]
    type = ElementAverageValue
    variable = effective_creep_strain
  [../]
  [./temperature]
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
[]

[Outputs]
  csv = true
[]
