[GlobalParams]
  gravity = '0 -0.001 0'
  convective_term = false
  integrate_p_by_parts = false
  u = vel_x
  v = vel_y
  pressure = p
[]

[Mesh]
  second_order = true
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 5
    ymax = 5
  [../]
  [./corner_node]
    type = ExtraNodesetGenerator
    new_boundary = top_right
    coord = '0 5'
    input = gen
  [../]
[]

[Variables]
  [./vel_x]
    order = SECOND
  [../]
  [./vel_y]
    order = SECOND
  [../]
  [./p]
  [../]
[]

[Kernels]
  [./mass]
    type = INSMass
    variable = p
  [../]
  [./x_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_x
    component = 0
  [../]
  [./y_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_y
    component = 1
  [../]
[]

[BCs]
  [./x_no_slip]
    type = DirichletBC
    variable = vel_x
    boundary = 'top bottom left right'
    value = 0.0
  [../]
  [./y_no_slip]
    type = DirichletBC
    variable = vel_y
    boundary = 'top bottom left right'
    value = 0.0
  [../]
  [./p_corner]
    type = DirichletBC
    boundary = top_right
    value = 0
    variable = p
  [../]
[]

[Materials]
  [./const]
    type = GenericConstantMaterial
    prop_names  = 'rho mu'
    prop_values = '100  1'
  [../]
[]

[Preconditioning]
  [./SMP_PJFNK]
    type = SMP
    full = true
    solve_type = NEWTON
  [../]
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-ksp_gmres_restart -pc_type -sub_pc_type -sub_pc_factor_levels'
  petsc_options_value = '300                bjacobi  ilu          4'
  line_search = none
  nl_rel_tol = 1e-12
  nl_max_its = 6
  l_tol = 1e-6
  l_max_its = 300
[]

[Outputs]
  exodus = true
  execute_on = TIMESTEP_END
[]
