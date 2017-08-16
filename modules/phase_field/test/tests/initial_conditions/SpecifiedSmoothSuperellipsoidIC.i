[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 25
  ny = 25
  xmax = 50
  ymax = 50
  elem_type = QUAD4
[]

[Variables]
  [./c]
    order = THIRD
    family = HERMITE
  [../]
[]

[ICs]
  [./c]
    type = SpecifiedSmoothSuperellipsoidIC
    variable = c
    x_positions = '15 35'
    y_positions = '25.0 25.0'
    z_positions = '0 0'
    as = '8.0 8.0'
    bs = '12.0 8.0'
    cs = '60.0 8.0'
    ns = '3.5 2.0'
    invalue = 1.0
    outvalue = -0.8
    int_width = 4.0
  [../]
[]

[Kernels]
  [./ie_c]
    type = TimeDerivative
    variable = c
  [../]
  [./CHSolid]
    type = CHMath
    variable = c
    mob_name = M
  [../]
  [./CHInterface]
    type = CHInterface
    variable = c
    kappa_name = kappa_c
    mob_name = M
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Materials]
  [./constant]
    type = GenericConstantMaterial
    prop_names  = 'M kappa_c'
    prop_values = '1.0 1.0'
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'
  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'

  l_max_its = 20
  l_tol = 1.0e-4
  nl_max_its = 40
  nl_rel_tol = 1e-9

  start_time = 0.0
  num_steps = 1
  dt = 2.0
[]

[Outputs]
  exodus = false
  [./out]
    type = Exodus
    refinements = 2
  [../]
[]
