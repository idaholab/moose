#
# Test consreved action for direct solve
#
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 16
  ny = 16
  xmax = 50
  ymax = 50
  elem_type = QUAD
[]

[Modules]
  [./PhaseField]
    [./Conserved]
      [./cv]
        solve_type = direct
        free_energy = F
        kappa = 2.0
        mobility = M
      [../]
    [../]
  [../]
[]

[ICs]
  [./InitialCondition]
    type = CrossIC
    x1 = 5.0
    y1 = 5.0
    x2 = 45.0
    y2 = 45.0
    variable = cv
  [../]
[]

[Materials]
  [./variable_mob]
    type = DerivativeParsedMaterial
    property_name = M
    coupled_variables = 'cv'
    expression = '0.1 + (1 + cv)/2'
  [../]
  [./free_energy]
    type = DerivativeParsedMaterial
    property_name = F
    coupled_variables = 'cv'
    expression = '(1-cv)^2 * (1+cv)^2'
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -sub_pc_type'
  petsc_options_value = 'asm lu'

  l_max_its = 30
  l_tol = 1.0e-4
  nl_max_its = 10
  nl_rel_tol = 1.0e-10

  start_time = 0.0
  num_steps = 5
  dt = 0.5
[]

[Outputs]
  [./out]
    type = Exodus
    refinements = 2
  [../]
[]
