[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 9
  ny = 6
  xmin = 10
  xmax = 40
  ymin = 15
  ymax = 35
  elem_type = QUAD
[]

[Modules]
  [./PhaseField]
    [./Conserved]
      [./c]
        free_energy = F
        mobility = 1.0
        kappa = 20.0
        coupled_variables = 'eta'
        solve_type = direct
      [../]
    [../]
    [./Nonconserved]
      [./eta]
        free_energy = F
        mobility = 1.0
        kappa = 20
        coupled_variables = 'c'
        family = HERMITE
        order = THIRD
      [../]
    [../]
  [../]
[]

[ICs]
  [./c_IC]
    type = BoundingBoxIC
    variable = c
    x1 = 10
    x2 = 25
    y1 = 15
    y2 = 35
    inside = 0.1
    outside = 0.9
  [../]
  [./eta_IC]
    type = ConstantIC
    variable = eta
    value = 0.5
  [../]
[]

[Materials]
  [./free_energy]
    type = DerivativeParsedMaterial
    property_name = F
    coupled_variables = 'eta c'
    expression = '(1 - eta)*10.0*(c - 0.1)^2 + eta*(8.0*(c - 0.9)^2) + 10.0*eta^2*(1-eta)^2'
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'
  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type -sub_pc_type'
  petsc_options_value = 'asm lu'

  l_max_its = 15
  l_tol = 1.0e-4

  nl_max_its = 10
  nl_rel_tol = 1.0e-11

  start_time = 0.0
  num_steps = 5
  dt = 0.05
[]

[Outputs]
  perf_graph = true
  [./out]
    type = Exodus
    refinements = 2
  [../]
[]
