[GlobalParams]
  gravity = '0 0 0'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1.1
  ymin = 0
  ymax = 1.1
  nx = 1
  ny = 1
  elem_type = QUAD4
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
  [../]

  [./vel_y]
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
  [./mass_pspg]
    type = INSMassPSPG
    variable = p
    u = vel_x
    v = vel_y
    p = p
  [../]

  # x-momentum, time
  [./x_momentum_time]
    type = INSMomentumTimeDerivative
    variable = vel_x
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

  # y-momentum, time
  [./y_momentum_time]
    type = INSMomentumTimeDerivative
    variable = vel_y
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

[]

[Materials]
  [./const]
    type = GenericConstantMaterial
    block = 0
    prop_names = 'rho mu'
    prop_values = '1.1  1.1'
  [../]
[]

[Functions]
  [./lid_function]
    # We pick a function that is exactly represented in the velocity
    # space so that the Dirichlet conditions are the same regardless
    # of the mesh spacing.
    type = ParsedFunction
    value = '4*x*(1-x)'
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
  num_steps = 1
  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_test_display'
  petsc_options_iname = '-snes_type'
  petsc_options_value = 'test'
[]

[Outputs]
  print_linear_residuals = false
  file_base = lid_driven_out_pspg
  exodus = true
[]

[ICs]
  [./vel_x]
    type = RandomIC
    variable = vel_x
    min = 0.1
    max = 0.9
  [../]
  [./vel_y]
    type = RandomIC
    variable = vel_y
    min = 0.1
    max = 0.9
  [../]
  [./p]
    type = RandomIC
    variable = p
    min = 0.1
    max = 0.9
  [../]
[]
