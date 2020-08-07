[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  second_order = true
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = false
[]

[Functions]
  [./pull]
    type = PiecewiseLinear
    x = '0 10'
    y = '0 1e-3'
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    add_variables = true
    use_finite_deform_jacobian = true
    generate_output = 'hydrostatic_stress'
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e10
    poissons_ratio = 0.3
  [../]

  [./elastic_strain]
    type = ComputeMultipleInelasticStress
    # inelastic_models = ''
    tangent_operator = nonlinear
  [../]

  [./creep_ten]
    type = PowerLawCreepStressUpdate
    coefficient = 10e-24
    n_exponent = 4
    activation_energy = 0
    base_name = creep_ten
  [../]
  [./creep_ten2]
    type = PowerLawCreepStressUpdate
    coefficient = 10e-24
    n_exponent = 4
    activation_energy = 0
    base_name = creep_ten2
  [../]
  [./creep_one]
    type = PowerLawCreepStressUpdate
    coefficient = 1e-24
    n_exponent = 4
    activation_energy = 0
    base_name = creep_one
  [../]
  [./creep_nine]
    type = PowerLawCreepStressUpdate
    coefficient = 9e-24
    n_exponent = 4
    activation_energy = 0
    base_name = creep_nine
  [../]
  [./creep_zero]
    type = PowerLawCreepStressUpdate
    coefficient = 0e-24
    n_exponent = 4
    activation_energy = 0
    base_name = creep_zero
  [../]
[]

[BCs]
  [./no_disp_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]

  [./no_disp_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]

  [./pull_disp_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = pull
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = 'none'
  nl_rel_tol = 1e-5

  num_steps = 5
  dt = 1e-1
[]

[Postprocessors]
  [./max_disp_x]
    type = ElementExtremeValue
    variable = disp_x
  [../]
  [./max_disp_y]
    type = ElementExtremeValue
    variable = disp_y
  [../]
  [./max_hydro]
    type = ElementAverageValue
    variable = hydrostatic_stress
  [../]
  [./dt]
    type = TimestepSize
  [../]
  [./num_lin]
    type = NumLinearIterations
    outputs = console
  [../]
  [./num_nonlin]
    type = NumNonlinearIterations
    outputs = console
  [../]
[]

[Outputs]
  csv = true
  perf_graph = true
[]

[Debug]
  show_var_residual_norms = true
[]
