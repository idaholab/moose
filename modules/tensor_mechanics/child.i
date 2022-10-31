beta = 0.25
gamma = 0.5

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    ymin = 0
    xmax = 0.2
    ymax = 0.5
    nx = 50
    ny = 150
    elem_type = QUAD4
  []
[]
[Variables]
  [disp_x]
    order = FIRST
  []
  [disp_y]
    order = FIRST
  []
[]

[AuxVariables] # variables that are calculated for output

  [solid_indicator]
    order = FIRST
    family = LAGRANGE
    [AuxKernel]
      type = ConstantAux
      variable = solid_indicator
      value = 0.0
      boundary = 'left right top'
      execute_on = 'INITIAL TIMESTEP_END'
    []
    initial_condition = 1.0
  []
[]

[Modules/TensorMechanics/DynamicMaster]
  [all]
    # displacements = 'disp_x disp_y'
    add_variables = true
    # new_system = true
    incremental = true
    strain = FINITE
    decomposition_method = EigenSolution
    # hht_alpha = 0.25
  []
[]

[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 10000.0
    poissons_ratio = 0.3
    use_displaced_mesh = true
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
  []
  [density]
    type = GenericConstantMaterial
    prop_names = 'density'
    prop_values = '1'
    use_displaced_mesh = true
  []
  [constant_stress]
    type = GenericConstantRankTwoTensor
    tensor_values = '100'
    tensor_name = test_tensor
  []
[]

[BCs]
  [hold_x]
    type = DirichletBC
    boundary = bottom
    variable = disp_x
    value = 0
    use_displaced_mesh = true
    # save_in = div_stess_x
  []
  [hold_y]
    type = DirichletBC
    boundary = bottom
    variable = disp_y
    value = 0
    use_displaced_mesh = true
    # save_in = div_stess_y
  []
  [Pressure]
    [push_left]
      boundary = left
      factor = 100
    []
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]
[Executioner]
  type = Transient
  end_time = 10
  dt = 1e-2
  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu       superlu_dist                  NONZERO               1e-15'
  nl_max_its = 40
  l_max_its = 15
  line_search = 'none'
  nl_abs_tol = 1e-5
  nl_rel_tol = 1e-4
  automatic_scaling = true
  [TimeIntegrator]
    type = NewmarkBeta
    beta = ${beta}
    gamma = ${gamma}
  []
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  print_linear_converged_reason = false
[]

# [MultiApps]
#   [fluid_domain]
#     type = TransientMultiApp
#     execute_on = "INITIAL TIMESTEP_END"
#     positions = '0 0 0'
#     input_files = real_fluid_multiapp.i
#     use_displaced_mesh = true
#   []
# []

# [Transfers]
#   [push_indicator]
#     type = MultiAppMeshFunctionTransfer
#     # Transfer to the sub-app from this app
#     to_multi_app = fluid_domain
#     # The name of the variable in this app
#     source_variable = solid_indicator
#     # The name of the auxiliary variable in the sub-app
#     variable = indicator
#     displaced_source_mesh = true
#     displaced_target_mesh = true
#     bbox_factor = 0.5
#     use_displaced_mesh = true
#   []
# []
