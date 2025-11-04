starting_point = 2e-1
offset = 1e-2

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  file = multiple_pairs.e
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    add_variables = true
    strain = FINITE
    generate_output = 'stress_xx'
  []
[]

[Materials]
  [stiffStuff]
    type = ComputeIsotropicElasticityTensor
    block = '1 2 3'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [stiffStuff_stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2 3'
  []
[]

[ICs]
  [disp_y]
    block = '2 3'
    variable = disp_y
    value = '${fparse starting_point + offset}'
    type = ConstantIC
  []
[]

[Contact]
  [action_name]
    primary = '20 20'
    secondary = '10 101'
    penalty = 1e7
    formulation = penalty
    tangential_tolerance = 0.0001
  []
[]

[BCs]
  [botx]
    type = DirichletBC
    variable = disp_x
    preset = false
    boundary = 40
    value = 0.0
  []
  [boty]
    type = DirichletBC
    variable = disp_y
    preset = false
    boundary = 40
    value = 0.0
  []
  [topy]
    type = FunctionDirichletBC
    variable = disp_y
    preset = false
    boundary = '30 301'
    function = '${starting_point} * cos(2 * pi / 40 * t) + ${offset}'
  []
  [leftx]
    type = FunctionDirichletBC
    variable = disp_x
    preset = false
    boundary = '50 501'
    function = '1e-2 * t'
  []
[]

[Executioner]
  type = Transient
  end_time = 60
  dt = 2.0
  dtmin = .1
  solve_type = 'PJFNK'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -pc_svd_monitor '
                  '-snes_linesearch_monitor'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount -mat_mffd_err'
  petsc_options_value = 'lu       NONZERO               1e-15                   1e-5'
  l_max_its = 30
  nl_max_its = 20
  nl_abs_tol = 1e-9
  line_search = 'none'
  snesmf_reuse_base = false
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  exodus = true
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  active = 'num_nl cumulative'
  [num_nl]
    type = NumNonlinearIterations
  []
  [cumulative]
    type = CumulativeValuePostprocessor
    postprocessor = num_nl
  []
[]

[VectorPostprocessors]
  [cont_press]
    type = NodalValueSampler
    variable = contact_pressure
    boundary = '10 101'
    sort_by = x
    execute_on = NONLINEAR
  []
[]
