[GlobalParams]
  gravity = '0 0 0'
  integrate_p_by_parts = true
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = .1
  ymin = 0
  ymax = .1
  nx = 40
  ny = 40
  elem_type = QUAD9
[]

[MeshModifiers]
  [./corner_node]
    type = AddExtraNodeset
    new_boundary = 'pinned_node'
    nodes = '0'
  [../]
[]

[Variables]
  [./vel_x]
    order = SECOND
    family = LAGRANGE
  [../]

  [./vel_y]
    order = SECOND
    family = LAGRANGE
  [../]

  [./p]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  # mass
  [./mass]
    type = INSMass
    variable = p
    u = vel_x
    v = vel_y
  [../]

  # x-momentum, time
  [./x_momentum_time]
    type = INSMomentumTimeDerivative
    variable = vel_x
  [../]
  [./x_momentum_time_supg]
    type = INSMomentumTimeDerivativeSUPG
    variable = vel_x
    u = vel_x
    v = vel_y
    component = 0
  [../]

  # x-momentum, space
  [./x_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_x
    u = vel_x
    v = vel_y
    p = p
    component = 0
  [../]
  [./x_supg]
    type = INSMomentumSUPG
    variable = vel_x
    u = vel_x
    v = vel_y
    p = p
    component = 0
  [../]

  # y-momentum, time
  [./y_momentum_time]
    type = INSMomentumTimeDerivative
    variable = vel_y
  [../]
  [./y_momentum_time_supg]
    type = INSMomentumTimeDerivativeSUPG
    variable = vel_y
    u = vel_x
    v = vel_y
    component = 1
  [../]

  # y-momentum, space
  [./y_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_y
    u = vel_x
    v = vel_y
    p = p
    component = 1
  [../]
  [./y_supg]
    type = INSMomentumSUPG
    variable = vel_y
    u = vel_x
    v = vel_y
    p = p
    component = 1
  [../]
[]

[BCs]
  [./x_no_slip]
    type = DirichletBC
    variable = vel_x
    boundary = 'bottom right left'
    value = 0.0
  [../]

  [./lid]
    type = FunctionDirichletBC
    variable = vel_x
    boundary = 'top'
    function = 'lid_function'
  [../]

  [./y_no_slip]
    type = DirichletBC
    variable = vel_y
    boundary = 'bottom right top left'
    value = 0.0
  [../]

  [./pressure_pin]
    type = DirichletBC
    variable = p
    boundary = 'pinned_node'
    value = 0
  [../]
[]

[Materials]
  [./const]
    type = GenericConstantMaterial
    block = 0
    prop_names = 'rho mu'
    prop_values = '1  1e-4'
  [../]
[]

[Functions]
  [./lid_function]
    type = ParsedFunction
    value = '400*x*(.1-x)'
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
    solve_type = 'NEWTON'
  [../]
[]

[Executioner]
  type = Transient
  dt = .5
  num_steps = 1
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu	  NONZERO'
  line_search = none
  nl_rel_tol = 1e-8
  nl_max_its = 20
  l_max_its = 30
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
[]
