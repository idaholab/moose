[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 30
  ny = 30
  xmax = 25.0
  ymax = 25.0
  elem_type = QUAD
[]

[Debug]
  show_actions = true
[]

[Modules]
  [./PhaseField]
    [./Conserved]
      [./c]
        solve_type = FORWARD_SPLIT
        mobility = 1.0
        kappa = kappa_c
        free_energy = F
      [../]
    [../]
  [../]
[]

[ICs]
  [./c_IC]
    type = CrossIC
    variable = c
    x1 = 0.0
    x2 = 25.0
    y1 = 0.0
    y2 = 25.0
  [../]
[]

[AuxVariables]
  [./local_energy]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxKernels]
  [./local_energy]
    type = TotalFreeEnergy
    variable = local_energy
    f_name = F
    kappa_names = kappa_c
    interfacial_vars = c
  [../]
[]

[Materials]
  [./kappa_c]
    type = GenericConstantMaterial
    prop_names = kappa_c
    prop_values = 2.0
  [../]
  [./free_energy]
    type = DerivativeParsedMaterial
    coupled_variables = c
    expression = '(1 - c)^2 * (1 + c)^2'
    property_name = F
  [../]
[]

[Postprocessors]
  [./total_free_energy]
    type = ElementIntegralVariablePostprocessor
    variable = local_energy
  [../]
  [./total_c]
    type = ElementIntegralVariablePostprocessor
    variable = c
    execute_on = 'initial TIMESTEP_END'
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

  l_max_its = 30
  l_tol = 1.0e-4
  nl_max_its = 10
  nl_rel_tol = 1.0e-10

  start_time = 0.0
  num_steps = 5
  dt = 0.7
[]

[Outputs]
  perf_graph = true
  exodus = true
[]
