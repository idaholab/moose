[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
  [./pot]
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
  [./adv_u]
    type = PotentialAdvection
    variable = u
    potential = pot
  [../]
  [./diff_pot]
    type = Diffusion
    variable = pot
  [../]
[]

[BCs]
  [./left]
    boundary = left
    type = DirichletBC
    value = 1
    variable = u
  [../]
  [./right]
    boundary = right
    type = DirichletBC
    variable = u
    value = 0
  [../]

# [./left_pot]
#     boundary = left
#     type = DirichletBC
#     value = 1
#     variable = pot
#   [../]
  [./left_pot]
    boundary = left
    type = ExampleShapeSideIntegratedBC
    value = 1
    variable = pot
    num_user_object = num_user_object
    denom_user_object = denom_user_object
    v = u
    Vb = 1
  [../]
  [./right_pot]
    boundary = right
    type = DirichletBC
    variable = pot
    value = 0
  [../]
  # [./right]
  #   boundary = right
  #   type = VacuumBC
  #   variable = u
  # [../]
[]

[UserObjects]
  [./num_user_object]
    type = NumShapeSiderUserObject
    u = u
  [../]
  [./denom_user_object]
    type = DenomShapeSideUserObject
    u = u
  [../]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  # petsc_options = '-snes_test_display'
  # petsc_options_iname = '-snes_type'
  # petsc_options_value = 'test'
  # dt = 0.1
  # num_steps = 2
[]

[Outputs]
  exodus = true
  print_perf_log = true
[]
