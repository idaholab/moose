[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 30
  ny = 6
  xmin = -15.0
  xmax = 15.0
  ymin = -3.0
  ymax = 3.0
  elem_type = QUAD4
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = RandomIC
      min = -0.1
      max =  0.1
    [../]
  [../]

  [./w]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Preconditioning]
  [./SMP]
   type = SMP
   off_diag_row = 'w c'
   off_diag_column = 'c w'
  [../]
[]

[Postprocessors]
  [./nfc]
    type = NodalFloodCount
    variable = c
    threshold = 0.8
  [../]
[]

[UserObjects]
  [./arnold]
    type = Terminator
    postprocessors = 'nfc'
    expression = 'nfc > 0'
  [../]
[]

[Kernels]
  [./cres]
    type = SplitCHMath
    variable = c
    kappa_name = kappa_c
    w = w
  [../]

  [./wres]
    type = SplitCHWRes
    variable = w
    mob_name = M
  [../]

  [./time]
    type = CoupledImplicitEuler
    variable = w
    v = c
  [../]
[]

[BCs]
  [./Periodic]
    [./c]
      variable = c
      auto_direction = 'x y'
    [../]
    [./w]
      variable = w
      auto_direction = 'x y'
    [../]
  [../]
[]

[Materials]
  [./constant]
    type = PFMobility
    block = 0
    mob = 1.0
    kappa = 2.0
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'BDF2'
  #petsc_options = '-snes_mf'

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  l_max_its = 30
  l_tol = 1.0e-3

  nl_max_its = 50
  nl_rel_tol = 1.0e-10

  dt = 10.0
  num_steps = 20
[]

[Outputs]
  file_base = out
  output_initial = true
  exodus = true
  [./console]
    type = Console
    perf_log = true
    linear_residuals = false
  [../]
[]
