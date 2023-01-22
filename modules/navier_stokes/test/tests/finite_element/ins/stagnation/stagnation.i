[GlobalParams]
  gravity = '0 0 0'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 2.0
  ymin = 0
  ymax = 2.0
  nx = 20
  ny = 20
  elem_type = QUAD9
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
    solve_type = Newton
  [../]
[]

[Executioner]
  type = Transient
  dt = 1.0
  dtmin = 1.e-6
  num_steps = 5
  l_max_its = 100
  nl_max_its = 15
  nl_rel_tol = 1.e-9

  petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type -sub_pc_factor_shift_type -ksp_gmres_restart'
  petsc_options_value = 'asm      2               lu           NONZERO                   1000'
  line_search = none
[]

[Variables]
  [./vel_x]
    family = LAGRANGE
    order = SECOND
  [../]
  [./vel_y]
    family = LAGRANGE
    order = SECOND
  [../]
  [./p]
    family = LAGRANGE
    order = FIRST
  [../]
[]

[BCs]
  [./u_in]
    type = FunctionDirichletBC
    boundary = 'top'
    variable = vel_x
    function = vel_x_inlet
  [../]
  [./v_in]
    type = FunctionDirichletBC
    boundary = 'top'
    variable = vel_y
    function = vel_y_inlet
  [../]
  [./vel_x_no_slip]
    type = DirichletBC
    boundary = 'left bottom'
    variable = vel_x
    value = 0
  [../]
  [./vel_y_no_slip]
    type = DirichletBC
    boundary = 'bottom'
    variable = vel_y
    value = 0
  [../]
  # Note: setting INSMomentumNoBCBC on the outlet boundary causes the
  # matrix to be singular.  The natural BC, on the other hand, is
  # sufficient to specify the value of the pressure without requiring
  # a pressure pin.
[]

[Functions]
  [./vel_x_inlet]
    type = ParsedFunction
    expression = 'k*x'
    symbol_names = 'k'
    symbol_values = '1'
  [../]
  [./vel_y_inlet]
    type = ParsedFunction
    expression = '-k*y'
    symbol_names = 'k'
    symbol_values = '1'
  [../]
[]


[Kernels]
  [./x_momentum_time]
    type = INSMomentumTimeDerivative
    variable = vel_x
  [../]
  [./y_momentum_time]
    type = INSMomentumTimeDerivative
    variable = vel_y
  [../]
  [./mass]
    type = INSMass
    variable = p
    u = vel_x
    v = vel_y
    pressure = p
  [../]
  [./x_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_x
    u = vel_x
    v = vel_y
    pressure = p
    component = 0
  [../]
  [./y_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_y
    u = vel_x
    v = vel_y
    pressure = p
    component = 1
  [../]
[]

[Materials]
  [./const]
    type = GenericConstantMaterial
    block = 0
    prop_names = 'rho mu'
    prop_values = '1 .01389' # 2/144
  [../]
[]

[Outputs]
  exodus = true
  [./out]
    type = CSV
    execute_on = 'final'
  [../]
[]

[VectorPostprocessors]
  [./nodal_sample]
    # Pick off the wall pressure values.
    type = NodalValueSampler
    variable = p
    boundary = 'bottom'
    sort_by = x
  [../]
[]
