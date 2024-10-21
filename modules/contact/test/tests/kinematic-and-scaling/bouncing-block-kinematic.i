starting_point = 2e-1
offset = 1e-2

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  file = long-bottom-block-no-lower-d-coarse.e
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[ICs]
  [./disp_y]
    block = 2
    variable = disp_y
    value = ${fparse starting_point + offset}
    type = ConstantIC
  [../]
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    add_variables = false
    use_automatic_differentiation = true
    strain = SMALL
  []
[]

[Materials]
  [elasticity]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e3
    poissons_ratio = 0.3
  []
  [stress]
    type = ADComputeLinearElasticStress
  []
[]

[Contact]
  [leftright]
    secondary = 10
    primary = 20
    model = frictionless
    formulation = kinematic
    penalty = 1e3
    normal_smoothing_distance = 0.1
  []
[]

[BCs]
  [./botx]
    type = DirichletBC
    variable = disp_x
    boundary = 40
    value = 0.0
  [../]
  [./boty]
    type = DirichletBC
    variable = disp_y
    boundary = 40
    value = 0.0
  [../]
  [./topy]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 30
    function = '${starting_point} * cos(2 * pi / 40 * t) + ${offset}'
  [../]
  [./leftx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 50
    function = '1e-2 * t'
  [../]
[]

[Executioner]
  type = Transient
  end_time = 100
  dt = 5
  dtmin = 5
  solve_type = 'PJFNK'
  petsc_options = '-snes_converged_reason -ksp_converged_reason'
  petsc_options_iname = '-pc_type -pc_hypre_type  -ksp_gmres_restart'
  petsc_options_value = 'hypre    boomeramg       200'
  l_max_its = 200
  nl_max_its = 20
  line_search = 'none'
  automatic_scaling = true
  verbose = true
  scaling_group_variables = 'disp_x disp_y'
  resid_vs_jac_scaling_param = 1
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  [exo]
    type = Exodus
  []
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Postprocessors]
  [nl]
    type = NumNonlinearIterations
  []
  [lin]
    type = NumLinearIterations
  []
  [tot_nl]
    type = CumulativeValuePostprocessor
    postprocessor = nl
  []
  [tot_lin]
    type = CumulativeValuePostprocessor
    postprocessor = lin
  []
[]
