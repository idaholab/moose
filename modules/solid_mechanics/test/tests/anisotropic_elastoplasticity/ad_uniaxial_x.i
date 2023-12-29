# This test simulates uniaxial tensile loading in x-direction.
# The slope of the stress vs. plastic strain is evaluated from
# the simulation and compared with the value calculated using
# the analytical expression. This test uses a material with li-
# near strain hardening.

# For uniaxial tensile loading in y-direction, the slope of the
# stress vs. plastic strain is (2K / (G + H)) where K is the ha-
# rdening constant, and G & H are the Hill's constant. For deta-
# ils on the derivation of the expression for slope please refer
# the documentation of this material.

# Slope obtained from this MOOSE test simulation:
#                = 1.791 x 10^9
# Slope obtained from analytical expression:
#                = 2 x 10^9 / (0.4 + 0.7) = 1.818 x 10^9

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
  []
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
  [plastic_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [plastic_strain_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [plastic_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [sigma_xx]
    order = CONSTANT
    family = MONOMIAL
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
    rank_two_tensor = plastic_strain
    variable = plastic_strain_xx
    index_i = 0
    index_j = 0
  []
  [plasticity_strain_xy]
    type = ADRankTwoAux
    rank_two_tensor = plastic_strain
    variable = plastic_strain_xy
    index_i = 0
    index_j = 1
  []
  [plasticity_strain_yy]
    type = ADRankTwoAux
    rank_two_tensor = plastic_strain
    variable = plastic_strain_yy
    index_i = 1
    index_j = 1
  []
  [sigma_xx]
    type = ADRankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
  []
[]

[Functions]
  [pull]
    type = PiecewiseLinear
    x = '0 1e1'
    y = '0 -2e8'
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = FINITE
    generate_output = 'elastic_strain_xx stress_xx strain_xx plastic_strain_xx'
    use_automatic_differentiation = true
    add_variables = true
  []
[]

[Materials]

  [elasticity_tensor]
    type = ADComputeElasticityTensor
    fill_method = orthotropic
    C_ijkl = '10.0e10 15.0e10 20.0e10 2.0e10 2.0e10 2.0e10 0.2 0.2 0.2 0.13333333333333333 0.1 0.15'
  []

  [elastic_strain]
    type = ADComputeMultipleInelasticStress
    inelastic_models = "trial_plasticity"
    max_iterations = 50
    absolute_tolerance = 1e-16
  []

  [hill_tensor]
    type = ADHillConstants
    # F G H L M N
    hill_constants = "0.6 0.4 0.7 1.5 1.5 1.5"
  []

  [trial_plasticity]
    type = ADHillElastoPlasticityStressUpdate
    hardening_constant = 10e9
    yield_stress = 60e6
    absolute_tolerance = 1e-15 # 1e-8
    relative_tolerance = 1e-13 # 1e-15
    # internal_solve_full_iteration_history = true
    max_inelastic_increment = 2.0e-5
    # internal_solve_output_on = on_error
  []
[]

[BCs]
  [no_disp_x]
    type = ADDirichletBC
    variable = disp_x
    boundary = left
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
    boundary = back
    value = 0.0
  []

  [Pressure]
    [Side1]
      boundary = right
      function = pull
    []
  []
[]

[Executioner]
  type = Transient

  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu     superlu_dist'

  nl_rel_tol = 1e-12
  nl_abs_tol = 1.0e-14
  l_max_its = 90
  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 30
    iteration_window = 9
    growth_factor = 1.05
    cutback_factor = 0.5
    timestep_limiting_postprocessor = matl_ts_min
    dt = 0.1
    time_t = '0 2.5 10'
    time_dt = '0.1 1.0e-2 1.0e-2'
  []
  start_time = 0
  end_time = 10.0
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
  [max_hydro]
    type = ElementAverageValue
    variable = hydrostatic_stress
  []
  [dt]
    type = TimestepSize
  []
  [plasticity_strain_xx]
    type = ElementalVariableValue
    variable = plastic_strain_xx
    execute_on = 'TIMESTEP_END'
    elementid = 0
  []
  [elastic_strain_xx]
    type = ElementalVariableValue
    variable = elastic_strain_xx
    execute_on = 'TIMESTEP_END'
    elementid = 0
  []
  [strain_xx]
    type = ElementalVariableValue
    variable = strain_xx
    execute_on = 'TIMESTEP_END'
    elementid = 0
  []
  [sigma_xx]
    type = ElementalVariableValue
    variable = stress_xx
    execute_on = 'TIMESTEP_END'
    elementid = 0
  []
[]

[Outputs]
  csv = true
  perf_graph = true
[]
