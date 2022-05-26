[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Problem]
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
  []
[]

[Modules/TensorMechanics/Master]
  [finite]
    add_variables = true
    strain = FINITE
    use_automatic_differentiation = true
  []
[]


[BCs]
  [no_x]
    type = ADDirichletBC
    variable = disp_x
    boundary = 'left'
    value = 0.0
  []
  [top]
    type = ADDirichletBC
    variable = disp_x
    boundary = 'top'
    value = 1e-4
  []
  [no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom'
    value = 0.0
  []
[]

[Materials]
  [elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e11
    poissons_ratio = 0.3
  []
  [elastic_stress]
    type = ADComputeMultipleInelasticStress
    inelastic_models = 'creep'
    outputs = all
  []
  [creep]
    type = ADPowerLawCreepTest
    coefficient = 10e-22
    n_exponent = 2
    activation_energy = 0
    internal_solve_full_iteration_history = true
    internal_solve_output_on = always
  []
[]

[Executioner]
  type = Transient
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = none
  num_steps = 1
[]

[Outputs]
[]
