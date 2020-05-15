offset = 0.001
vy = 0.1

refine = 0

[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
[]

[Mesh]
  [./original_file_mesh]
    type = FileMeshGenerator
    # file = long_short_blocks.e
    file = two_blocks_non-match.e
  [../]
  uniform_refine =  ${refine}
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    incremental = true
    add_variables = true
    block = '1 2'

    # use_displaced_mesh = true
  [../]
[]

# [AuxVariables]
#   [./penetration]
#     order = FIRST
#     family = LAGRANGE
#   [../]
# []

[Functions]
  [./horizontal_movement]
    type = ParsedFunction
    value = '0.002-0.003*exp(-4.05*t)'
  [../]
  [./vertical_movement]
    type = ParsedFunction
    value = '${vy}*t+${offset}'
  [../]
[]

# [AuxKernels]
#   [./penetration]
#     type = PenetrationAux
#     variable = penetration
#     boundary = 10
#     paired_boundary = 20
#     order = FIRST
#   [../]
# []

[BCs]
  [./push_left_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 30
    function = horizontal_movement
  [../]
  [./fix_right_x]
    type = DirichletBC
    variable = disp_x
    boundary = 40
    value = 0.0
  [../]
  [./fix_right_y]
    type = DirichletBC
    variable = disp_y
    boundary = '40'
    value = 0.0
  [../]
  [./push_left_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = '30'
    function = vertical_movement
  [../]
[]

[Materials]
  [./elasticity_tensor_left]
    type = ComputeIsotropicElasticityTensor
    block = 1
    youngs_modulus = 1.0e6
    poissons_ratio = 0.3
  [../]
  [./stress_left]
    type = ComputeFiniteStrainElasticStress
    block = 1
  [../]

  [./elasticity_tensor_right]
    type = ComputeIsotropicElasticityTensor
    block = 2
    youngs_modulus = 1.0e6
    poissons_ratio = 0.3
  [../]
  [./stress_right]
    type = ComputeFiniteStrainElasticStress
    block = 2
  [../]
[]

[Contact]
  [leftright]
    mesh = original_file_mesh
    slave = 10
    master = 20

    use_dual = true

    formulation = mortar
    model = frictionless

    # friction_coefficient = 0.2
  [../]
[]

[ICs]
  [./disp_y]
    block = 1
    variable = disp_y
    value = ${offset}
    type = ConstantIC
  [../]
  [./disp_x]
    block = 1
    variable = disp_x
    value = ${offset}
    type = ConstantIC
  [../]
[]

[Preconditioning]
  [./FDP]
    type = FDP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'

  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_ksp_ew -snes_fd'

  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu NONZERO   1e-15'

  # petsc_options_iname = '-pc_type -ksp_view_mat'
  # petsc_options_value = 'svd ascii:dual_matrix_ref${refine}.m:ascii_matlab'

  dt = 0.1
  dtmin = 1e-4
  end_time = 0.2

  # l_tol = 1e-8
  l_max_its = 100

  # nl_rel_tol = 1e-6
  # nl_abs_tol = 1e-8
  nl_max_its = 30
  nl_rel_tol = 1e-6
[]

[Outputs]
  file_base = ./contact_sliding_LU_refine_${refine}_out
  [./exodus]
    type = Exodus
  [../]
  [./console]
    type = Console
    max_rows = 5
  [../]
  # [./csv]
  #   type = CSV
  # [../]
  # [dof_map]
  #   type = DOFMap
  #   execute_on = 'initial'
  # []
  [./pgragh]
    type = PerfGraphOutput
    execute_on = FINAL
    level = 1
  [../]
[]


[Postprocessors]
  [./nl_its]
    type = NumNonlinearIterations
    execute_on = 'initial timestep_end'
  [../]
  [./lin_its]
    type = NumLinearIterations
    execute_on = 'initial timestep_end'
  [../]
  [./cumul_nl]
    type = CumulativeValuePostprocessor
    postprocessor = nl_its
  [../]
  [./cumul_lin]
    type = CumulativeValuePostprocessor
    postprocessor = lin_its
  [../]
  [./contact]
    type = ContactDOFSetSize
    variable = leftright_normal_lm
    subdomain = leftright_slave_subdomain
    execute_on = 'nonlinear timestep_end'
  []
[]
