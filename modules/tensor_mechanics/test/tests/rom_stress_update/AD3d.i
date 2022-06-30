p = 1e5
E = 3.3e11
stress_unit = 'Pa'

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 2
  nz = 2
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
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
    generate_output = 'vonmises_stress'
    use_automatic_differentiation = true
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
  [symmz]
    type = ADDirichletBC
    variable = disp_z
    boundary = back
    value = 0
  []
  [pressure_x]
    type = ADPressure
    variable = disp_x
    boundary = right
    factor = ${p}
  []
  [pressure_y]
    type = ADPressure
    variable = disp_y
    boundary = top
    factor = -${p}
  []
  [pressure_z]
    type = ADPressure
    variable = disp_z
    boundary = front
    factor = -${p}
  []
[]

[Materials]
  [elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = ${E}
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
    stress_unit = ${stress_unit}
  []
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
[]

[Outputs]
  csv = true
[]
