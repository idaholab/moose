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
  [temperature]
    initial_condition = 900.0
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = FINITE
    add_variables = true
    use_automatic_differentiation = true
    generate_output = vonmises_stress
  []
[]

[BCs]
  [symmy]
    type = ADDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [symmx]
    type = ADDirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [pressure_x]
    type = ADPressure
    variable = disp_x
    boundary = right
    function = t
    factor = 3.1675e5
  []
  [pressure_y]
    type = ADPressure
    variable = disp_y
    boundary = top
    function = t
    factor = 6.336e5
  []
[]

[Materials]
  [elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 3.30e11
    poissons_ratio = 0.3
  []
  [stress]
    type = ADComputeMultipleInelasticStress
    inelastic_models = rom_stress_prediction
  []
  [rom_stress_prediction]
    type = ADSS316HLAROMANCEStressUpdateTest
    temperature = temperature
    initial_cell_dislocation_density = 6.0e12
    initial_wall_dislocation_density = 4.4e11
    outputs = all
  []
[]

[Executioner]
  type = Transient

  solve_type = 'NEWTON'

  nl_abs_tol = 1e-12
  automatic_scaling = true
  compute_scaling_once = false

  num_steps = 5
  dt = 2
[]

[Postprocessors]
  [effective_strain_avg]
    type = ElementAverageValue
    variable = effective_creep_strain
  []
  [temperature]
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
