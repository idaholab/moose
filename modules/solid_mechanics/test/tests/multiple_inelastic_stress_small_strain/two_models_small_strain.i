# Test for ComputeMultipleInelasticStressSmallStrain with two inelastic models
# This test combines power law creep and isotropic plasticity using small strain formulation
#
# A single element is loaded in tension. This verifies that the iterative
# algorithm in ComputeMultipleInelasticStressSmallStrain correctly handles
# multiple inelastic models and converges to the correct combined inelastic strain.

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

[Functions]
  [top_pull]
    type = ParsedFunction
    expression = 't * 0.005'
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = SMALL
    incremental = false
    add_variables = true
    generate_output = 'stress_yy'
  []
[]

[BCs]
  [y_pull_function]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = top_pull
  []
  [x_sides]
    type = DirichletBC
    variable = disp_x
    boundary = 'left right'
    value = 0.0
  []
  [y_bot]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
  [z_sides]
    type = DirichletBC
    variable = disp_z
    boundary = 'back front'
    value = 0.0
  []
[]

[AuxVariables]
  [combined_inelastic_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [combined_inelastic_strain_yy]
    type = RankTwoAux
    rank_two_tensor = combined_inelastic_strain
    variable = combined_inelastic_strain_yy
    index_i = 1
    index_j = 1
    execute_on = timestep_end
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2.0e5
    poissons_ratio = 0.3
  []

  [creep]
    type = PowerLawCreepStressUpdate
    coefficient = 1.0e-15
    n_exponent = 5.0
    m_exponent = 0.0
    activation_energy = 0.0
  []

  [plasticity]
    type = IsotropicPlasticityStressUpdate
    yield_stress = 150.0
    hardening_constant = 500.0
  []

  [stress]
    type = ComputeMultipleInelasticStressSmallStrain
    inelastic_models = 'creep plasticity'
    max_iterations = 50
    relative_tolerance = 1e-8
    absolute_tolerance = 1e-10
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
  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  l_max_its = 20
  nl_max_its = 20
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10

  start_time = 0.0
  end_time = 2.0
  dt = 0.5
[]

[Outputs]
  exodus = true
  csv = true
[]

[Postprocessors]
  [stress_yy]
    type = ElementAverageValue
    variable = stress_yy
  []
  [combined_strain_yy]
    type = ElementAverageValue
    variable = combined_inelastic_strain_yy
  []
[]
