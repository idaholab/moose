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
  [d]
    [InitialCondition]
      type = CrossIC
      x1 = 0.0
      x2 = 30.0
      y1 = 0.0
      y2 = 30.0
    []
  []
  [w]
  []
[]

[AuxVariables]
  [c]
  []
[]

[AuxKernels]
  [c]
    type = ProjectionAux
    variable = c
    v = d
    execute_on = 'INITIAL TIMESTEP_END FINAL'
  []
[]

[Preconditioning]
  active = 'SMP'
  [PBP]
    type = PBP
    solve_order = 'w d'
    preconditioner = 'AMG ASM'
    off_diag_row = 'd '
    off_diag_column = 'w '
  []

  [SMP]
    type = SMP
    off_diag_row = 'w d'
    off_diag_column = 'd w'
  []
[]

[Kernels]
  [cres]
    type = SplitCHParsed
    variable = d
    kappa_name = kappa_c
    w = w
    f_name = F
  []

  [wres]
    type = SplitCHWRes
    variable = w
    mob_name = M
  []

  [time]
    type = CoupledTimeDerivative
    variable = w
    v = d
  []
[]

[BCs]
  [Periodic]
    [top_bottom]
      primary = 0
      secondary = 2
      translation = '0 30.0 0'
    []

    [left_right]
      primary = 1
      secondary = 3
      translation = '-30.0 0 0'
    []
  []
[]

[Materials]
  [constant]
    type = GenericConstantMaterial
    prop_names = 'M kappa_c'
    prop_values = '1.0 2.0'
  []
  [free_energy]
    type = MathEBFreeEnergy
    property_name = F
    c = d
  []
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
  hide = d
[]
