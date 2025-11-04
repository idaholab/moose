starting_point = 2e-1
offset = 1e-2

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = long-bottom-block-no-lower-d.e
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[ICs]
  [disp_y]
    block = 2
    variable = disp_y
    value = '${fparse starting_point + offset}'
    type = ConstantIC
  []
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
    youngs_modulus = 1e0
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
    formulation = mortar
    c_normal = 1e-1
  []
[]

[BCs]
  [botx]
    type = DirichletBC
    variable = disp_x
    boundary = 40
    value = 0.0
  []
  [boty]
    type = DirichletBC
    variable = disp_y
    boundary = 40
    value = 0.0
  []
  [topy]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 30
    function = '${starting_point} * cos(2 * pi / 40 * t) + ${offset}'
  []
  [leftx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 50
    function = '1e-2 * t'
  []
[]

[Executioner]
  type = Transient
  num_steps = 40
  end_time = 200
  dt = 5
  dtmin = 5
  solve_type = 'PJFNK'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_ksp_ew'
  petsc_options_iname = '-pc_type -mat_mffd_err -pc_factor_shift_type'
  petsc_options_value = 'lu       1e-5          NONZERO'
  l_max_its = 30
  nl_max_its = 20
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10
  line_search = 'none'
  snesmf_reuse_base = true
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
  [smp]
    type = SMP
    full = true
  []
[]
