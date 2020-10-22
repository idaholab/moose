# 1x1x1 unit cube with uniform pressure on top face

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
[]

[Variables]
  [./temp]
    initial_condition = 1000.0
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    incremental = true
    add_variables = true
    generate_output = 'stress_yy creep_strain_xx creep_strain_yy creep_strain_zz elastic_strain_yy'
    use_automatic_differentiation = true
  [../]
[]

[Kernels]
  [./heat]
    type = ADMatDiffusion
    variable = temp
    diffusivity = 100
  [../]
  [./heat_ie]
    type = ADTimeDerivative
    variable = temp
  [../]
[]

[BCs]
  [./u_top_pull]
    type = ADPressure
    variable = disp_y
    component = 1
    boundary = top
    constant = -10.0e6
  [../]
  [./u_bottom_fix]
    type = ADDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./u_yz_fix]
    type = ADDirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./u_xy_fix]
    type = ADDirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  [../]
  [./temp_fix]
    type = DirichletBC
    variable = temp
    boundary = 'bottom top'
    value = 1000.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 2e11
    poissons_ratio = 0.3
    constant_on = SUBDOMAIN
  [../]
  [./radial_return_stress]
    type = ADComputeMultipleInelasticStress
    inelastic_models = 'power_law_creep'
    use_old_elasticity_tensor = true
  [../]
  [./power_law_creep]
    type = ADPowerLawCreepStressUpdate
    coefficient = 1.0e-15
    n_exponent = 4
    activation_energy = 3.0e5
    temperature = temp
    use_old_elasticity_tensor = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = 'none'

  num_steps = 10
  dt = 0.1
[]

[Outputs]
  exodus = true
[]
