[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 50
  nz = 0
  xmax = 100
  ymax = 100
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[ICs]
  [./c]
    type = MultiSmoothSuperellipsoidIC
    variable = c
    invalue = 1.0
    outvalue = 0.1
    bubspac = 10
    numbub = 5
    semiaxis_b_variation = 0.25
    semiaxis_variation_type = uniform
    semiaxis_a_variation = 0.2
    semiaxis_a = 7
    semiaxis_b = 10
  [../]
[]

[Executioner]
  # Preconditioned JFNK (default)
  type = Transient
  scheme = bdf2
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart -mat_mffd_type'
  petsc_options_value = 'hypre boomeramg 31 ds'
  l_max_its = 20
  l_tol = 1e-4
  nl_max_its = 20
  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-11
  start_time = 0.0
  num_steps = 1
  dt = 100.0
  enable = false
  [./Adaptivity]
    refine_fraction = .5
  [../]
[]

[Outputs]
  exodus = true
[]

[Problem]
  type = FEProblem
  solve = false
[]

