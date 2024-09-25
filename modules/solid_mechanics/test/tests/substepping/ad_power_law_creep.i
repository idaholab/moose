[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
  []
[]

[AuxVariables]
  [temp]
    initial_condition = 1000.0
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = FINITE
    add_variables = true
    generate_output = 'stress_zz elastic_strain_zz creep_strain_zz'
    use_automatic_differentiation = true
  []
[]

[Functions]
  [front_pull]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 1'
    scale_factor = 0.5
  []
[]

[BCs]
  [u_front_pull]
    type = ADFunctionDirichletBC
    variable = disp_z
    boundary = front
    function = front_pull
  []
  [uz_back_fix]
    type = ADDirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  []
  [u_yz_fix]
    type = ADDirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []
  [u_xz_fix]
    type = ADDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
[]

[Materials]
  [elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 2e11
    poissons_ratio = 0.3
  []
  [radial_return_stress]
    type = ADComputeMultipleInelasticStress
    inelastic_models = 'power_law_creep'
  []
  [power_law_creep]
    type = ADPowerLawCreepStressUpdate
    coefficient = 1.0e-15
    n_exponent = 4
    activation_energy = 0.0
    temperature = temp
    # options for using substepping
    substep_strain_tolerance = 0.1
    max_inelastic_increment = 0.01
    # output radial return materials
    outputs = exodus
    output_properties = "effective_creep_strain_rate effective_creep_strain"
  []
[]

[Executioner]
  type = Transient

  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type '
  petsc_options_value = 'lu     '

  line_search = 'none'

  nl_max_its = 10
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10

  end_time = 0.2
  dt = 0.1

  [Predictor]
    type = SimplePredictor
    scale = 1
  []
[]

[Outputs]
  exodus = true
[]
