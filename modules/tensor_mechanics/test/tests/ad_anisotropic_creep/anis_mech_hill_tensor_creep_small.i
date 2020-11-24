[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 12
  ny = 12
  second_order = true
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = false
[]

[AuxVariables]
  [hydrostatic_stress]
    order = CONSTANT
    family = MONOMIAL
  []
[]
[Variables]
  [disp_x]
    order = SECOND
    scaling = 1e-10
  []
  [disp_y]
    order = SECOND
    scaling = 1e-10
  []
[]
[AuxKernels]
  [hydrostatic_stress]
    type = ADRankTwoScalarAux
    variable = hydrostatic_stress
    rank_two_tensor = stress
    scalar_type = Hydrostatic
  []
[]

[Functions]
  [pull]
    type = PiecewiseLinear
    x = '0 1e3'
    y = '0 1e2'
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = FINITE
    add_variables = true
    incremental = true
    generate_output = 'elastic_strain_xx elastic_strain_yy elastic_strain_xy stress_xx stress_xy stress_yy'
    use_automatic_differentiation = true
  []
[]

[Materials]
  [elasticity_tensor]
    type = ADComputeElasticityTensor
    fill_method = orthotropic
    C_ijkl = '2.0e3 2.0e5 2.0e3 0.71428571e3 0.71428571e3 0.71428571e3 0.4 0.2 0.004 0.004 0.2 0.4'
  []

#  [./elasticity_tensor]
#      type = ADComputeIsotropicElasticityTensor
#      youngs_modulus = 206800
#      poissons_ratio = 0.0
#    [../]

#  [stress_]
#     type = ADComputeFiniteStrainElasticStress
#  []

  [elastic_strain]
    type = ADComputeMultipleInelasticStress
    inelastic_models = "trial_creep trial_creep_two"
    max_iterations = 500
    absolute_tolerance = 1e-05
  []

  [trial_creep]
    type = ADTransverselyIsotropicCreepStressUpdate
    coefficient = 3e-18
    n_exponent = 5
    m_exponent = 0
    activation_energy = 0
    # internal_solve_output_on = always
    # F G H L M N
    hill_constants = "0.5 0.5 0.3866 1.6413 1.6413 1.2731"
    base_name = trial_creep
  []

  [trial_creep_two]
    type = ADTransverselyIsotropicCreepStressUpdate
    coefficient = 3e-18
    n_exponent = 5
    m_exponent = 0
    activation_energy = 0
    # internal_solve_output_on = always
    # F G H L M N
    hill_constants = "0.5 0.5 0.3866 1.6413 1.6413 1.2731"
    base_name = trial_creep_two
  []
  [creep_one]
    type = ADPowerLawCreepStressUpdate
    coefficient = 1e-18
    n_exponent = 5
    activation_energy = 0
    base_name = creep_one
  []
  [creep_nine]
    type = ADPowerLawCreepStressUpdate
    coefficient = 9e-24
    n_exponent = 4
    activation_energy = 0
    base_name = creep_nine
  []
[]

[BCs]
  [no_disp_x]
    type = ADDirichletBC
    variable = disp_x
    boundary = bottom
    value = 0.0
  []

  [no_disp_y]
    type = ADDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []

  [Pressure]
    [Side1]
      boundary = top
      function = pull
    []
  []

  #  [./pull_disp_y]
  #    type = ADFunctionDirichletBC
  #    variable = disp_y
  #    boundary = top
  #    function = pull
  #  [../]
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient

  solve_type = NEWTON
  petsc_options_iname = '-ksp_gmres_restart -pc_type -sub_pc_type'
  petsc_options_value = '101                asm      lu'

  line_search = 'none'
  nl_rel_tol = 1e-06
  nl_abs_tol = 1.0e-14
  l_max_its = 90
  num_steps = 40
  dt = 5.0e2
  automatic_scaling = true
[]

[Postprocessors]
  [max_disp_x]
    type = ElementExtremeValue
    variable = disp_x
  []
  [max_disp_y]
    type = ElementExtremeValue
    variable = disp_y
  []
  [max_hydro]
    type = ElementAverageValue
    variable = hydrostatic_stress
  []
  [dt]
    type = TimestepSize
  []
  [num_lin]
    type = NumLinearIterations
    outputs = console
  []
  [num_nonlin]
    type = NumNonlinearIterations
    outputs = console
  []
[]

[Outputs]
  csv = true
  exodus = true
[]
