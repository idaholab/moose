[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 30
  ny = 30
  xmin = 0.0
  xmax = 30.0
  ymin = 0.0
  ymax = 30.0
  elem_type = QUAD4
[]

[Variables]
  [./c]
    [./InitialCondition]
      type = CrossIC
      x1 = 0.0
      x2 = 30.0
      y1 = 0.0
      y2 = 30.0
    [../]
  [../]
  [./w]
  [../]
[]

[Preconditioning]
active = 'SMP'
  [./PBP]
   type = PBP
   solve_order = 'w c'
   preconditioner = 'AMG ASM'
   off_diag_row = 'c '
   off_diag_column = 'w '
  [../]

  [./SMP]
   type = SMP
   coupled_groups = 'c,w'
  [../]
[]

[Kernels]
  [./cres]
    type = SplitCHParsed
    variable = c
    kappa_name = kappa_c
    w = w
    f_name = F
  [../]

  [./wres]
    type = SplitCHWRes
    variable = w
    mob_name = M
  [../]

  [./time]
    type = CoupledTimeDerivative
    variable = w
    v = c
  [../]
[]

[BCs]
  [./Periodic]
    [./top_bottom]
      primary = 0
      secondary = 2
      translation = '0 30.0 0'
    [../]

    [./left_right]
      primary = 1
      secondary = 3
      translation = '-30.0 0 0'
    [../]
  [../]
[]

[Materials]
  [./constant]
    type = GenericConstantMaterial
    prop_names  = 'M kappa_c'
    prop_values = '1.0 2.0'
  [../]

  [./free_energy]
    type = MathFreeEnergy
    property_name = F
    c = c
    derivative_order = 2
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'BDF2'
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  l_max_its = 30
  l_tol = 1.0e-3

  nl_max_its = 50
  nl_rel_tol = 1.0e-10

  dt = 10.0
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
