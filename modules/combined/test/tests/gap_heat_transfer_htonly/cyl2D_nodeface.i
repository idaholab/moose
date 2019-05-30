[GlobalParams]
  # order = SECOND
  # family = LAGRANGE
  displacements = 'disp_x disp_y'
[]

[Mesh]
  file = cyl2D.e
[]

[MeshModifiers]
  [./bottom]
    type = BoundingBoxNodeSet
    new_boundary = 10
    bottom_left = '-1 -1 0'
    top_right = '4 0.01 0'
  [../]
  [./left]
    type = BoundingBoxNodeSet
    new_boundary = 11
    bottom_left = '-1 -1 0'
    top_right = '0.01 4 0'
  [../]
  [slave]
    type = LowerDBlockFromSideset
    sidesets = '2'
    new_block_id = 200
    new_block_name = 'slave_lower'
  []
  [master]
    type = LowerDBlockFromSideset
    sidesets = '3'
    new_block_id = 300
    new_block_name = 'master_lower'
  []
[]

[Functions]
  [./temp]
    type = PiecewiseLinear
    x = '0   1'
    y = '0 25'
    # y = '0 250'
    # y = '0 0'
  [../]
[]

[Variables]
  [./temp]
    order = SECOND
    family = LAGRANGE
    initial_condition = 0
    block = '1 2'
  [../]
  [./lm]
    order = SECOND
    family = LAGRANGE
    block = 'slave_lower'
  [../]
[]

[Modules]
  [./TensorMechanics]
    [./Master]
      [./all]
        add_variables = true
        strain = SMALL
        block = '1 2'
        eigenstrain_names = thermal_expansion
      [../]
    [../]
  [../]
[]

# [AuxVariables]
#   [./disp_x]
#     order = SECOND
#     family = LAGRANGE
#     block = '1 2'
#   [../]
#   [./disp_y]
#     order = SECOND
#     family = LAGRANGE
#     block = '1 2'
#   [../]
# []

[Kernels]
  [./heat_conduction]
    type = HeatConduction
    block = '1 2'
    variable = temp
  [../]
[]


[Materials]
  [./heat1]
    type = HeatConductionMaterial
    block = '1'
    specific_heat = 1.0
    thermal_conductivity = 10.0
  [../]
  [./heat2]
    type = HeatConductionMaterial
    block = '2'
    specific_heat = 1.0
    thermal_conductivity = 1.0
  [../]

  [./stress]
    type = ComputeLinearElasticStress
    block = '1 2'
  [../]
  [./elasticity]
    type = ComputeIsotropicElasticityTensor
    block = '1 2'
    youngs_modulus = 1
    poissons_ratio = 0.33
  [../]
  [./thermal_expansion]
    type = ComputeThermalExpansionEigenstrain
    block = '1 2'
    eigenstrain_name = thermal_expansion
    thermal_expansion_coeff = 0.01
    stress_free_temperature = 0
    temperature = temp
  [../]
[]

[Constraints]
  [./ced]
    type = GapConductanceConstraint
    variable = lm
    slave_variable = temp
    k = 0.05
    use_displaced_mesh = true
    master_boundary = 3
    master_subdomain = 300
    slave_boundary = 2
    slave_subdomain = 200
    displacements = 'disp_x disp_y'
    compute_lm_residuals = false
  [../]
  [./lm]
    type = LagrangeNodeFace
    master = 3
    slave = 2
    variable = lm
    master_variable = temp
    k = 0.005
  [../]
[]

# [AuxKernels]
#   [./disp_x]
#     type = FunctionAux
#     variable = disp_x
#     function = x*(t+1)
#     block = '1 2'
#   [../]
#   [./disp_y]
#     type = FunctionAux
#     variable = disp_y
#     function = y*(t+1)
#     block = '1 2'
#   [../]
# []

[BCs]
  [./mid]
    type = FunctionPresetBC
    boundary = 1
    variable = temp
    function = temp
  [../]
  [./temp_far_right]
    type = PresetBC
    boundary = 4
    variable = temp
    value = 0
  [../]
  [./bottom]
    type = PresetBC
    boundary = 10
    variable = disp_y
    value = 0
  [../]
  [./left]
    type = PresetBC
    boundary = 11
    variable = disp_x
    value = 0
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
  line_search = bt
  solve_type = NEWTON

  petsc_options = '-ksp_converged_reason -snes_converged_reason -snes_linesearch_monitor -pc_svd_monitor'
  # petsc_options_iname = '-pc_type -pc_factor_mat_solver_package -mat_mffd_err -pc_factor_shift_type -pc_factor_shift_amount' # -mat_mffd_err
  # petsc_options_value = 'lu       superlu_dist                  1e-8          NONZERO               1e-15'

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'svd'

  dt = 0.1
  dtmin = 0.01
  end_time = 1

  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-7

  # [./Quadrature]
  #   order = fifth
  #   side_order = seventh
  # [../]
[]

[Outputs]
  exodus = true
  [./Console]
    type = Console
  [../]
  [./ee]
    type = Exodus
    execute_on = 'NONLINEAR'
  [../]
[]

[Debug]
  show_var_residual_norms = true
[]

[Postprocessors]
  [./temp_left]
    type = SideAverageValue
    boundary = 2
    variable = temp
  [../]

  [./temp_right]
    type = SideAverageValue
    boundary = 3
    variable = temp
  [../]

  [./flux_left]
    type = SideFluxIntegral
    variable = temp
    boundary = 2
    diffusivity = thermal_conductivity
  [../]

  [./flux_right]
    type = SideFluxIntegral
    variable = temp
    boundary = 3
    diffusivity = thermal_conductivity
  [../]
[]
