[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 3
  ny = 3
  nz = 3
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
[]

[AuxVariables]
  [hydrostatic_stress]
    order = CONSTANT
    family = MONOMIAL
  []
  [plasticity_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [plasticity_strain_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [plasticity_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [elastic_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  []
[]
[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]
[AuxKernels]
  [hydrostatic_stress]
    type = ADRankTwoScalarAux
    variable = hydrostatic_stress
    rank_two_tensor = stress
    scalar_type = Hydrostatic
  []
  [plasticity_strain_xx]
    type = ADRankTwoAux
    rank_two_tensor = trial_plasticity_plasticity_strain
    variable = plasticity_strain_xx
    index_i = 0
    index_j = 0
  []
  [plasticity_strain_xy]
    type = ADRankTwoAux
    rank_two_tensor = trial_plasticity_plasticity_strain
    variable = plasticity_strain_xy
    index_i = 0
    index_j = 1
  []
  [plasticity_strain_yy]
    type = ADRankTwoAux
    rank_two_tensor = trial_plasticity_plasticity_strain
    variable = plasticity_strain_yy
    index_i = 1
    index_j = 1
  []
  [elastic_strain_yy]
    type = ADRankTwoAux
    rank_two_tensor = elastic_strain
    variable = elastic_strain_yy
    index_i = 1
    index_j = 1
  []
[]

[Functions]
  [pull]
    type = PiecewiseLinear
    x = '0 1e3 1e8'
    y = '0 1e2 1e2'
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
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 206800
    poissons_ratio = 0.3
  []

  #  [stress_]
  #     type = ADComputeFiniteStrainElasticStress
  #  []

  [elastic_strain]
    type = ADComputeMultipleInelasticStress
    inelastic_models = "trial_plasticity"
    max_iterations = 50
    absolute_tolerance = 1e-09
  []

  [trial_plasticity]
    type = ADTransverselyIsotropicPlasticityStressUpdate
    # internal_solve_output_on = always
    # F G H L M N
    hardening_constant = 4000
    yield_stress = 2.0
    hill_constants = "1.0 4.0 5.0 0.5 0.5 0.5"
    absolute_tolerance = 1e-14
    base_name = trial_plasticity
    internal_solve_full_iteration_history = true
    max_inelastic_increment = 1.0e-6
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

  [no_disp_z]
    type = ADDirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  []

  [Pressure]
    [Side1]
      boundary = top
      function = pull
    []
  []

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

  nl_rel_tol = 1e-07
  nl_abs_tol = 1.0e-14
  l_max_its = 90
  num_steps = 40
  # dt = 5.0e0
  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 30
    iteration_window = 9
    growth_factor = 1.0
    cutback_factor = 0.5
    timestep_limiting_postprocessor = matl_ts_min
    dt = 5.0e-1
  []
  start_time = 0
  automatic_scaling = true
[]

[Postprocessors]
  [matl_ts_min]
    type = MaterialTimeStepPostprocessor
  []
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
  [plasticity_strain]
    type = ElementAverageValue
    variable = plasticity_strain_yy
  []
  [elastic_strain]
    type = ElementAverageValue
    variable = elastic_strain_yy
  []
[]

[Outputs]
  csv = true
  exodus = true
  perf_graph = true
[]
