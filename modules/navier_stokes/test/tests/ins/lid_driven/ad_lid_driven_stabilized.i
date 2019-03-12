[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1.0
  ymin = 0
  ymax = 1.0
  nx = 64
  ny = 64
[]

[MeshModifiers]
  [./corner_node]
    type = AddExtraNodeset
    new_boundary = 'pinned_node'
    nodes = '0'
  [../]
[]

[AuxVariables]
  [vel_x]
  []
  [vel_y]
  []
[]

[AuxKernels]
  [vel_x]
    type = VectorVariableComponentAux
    variable = vel_x
    vector_variable = velocity
    component = 'x'
  []
  [vel_y]
    type = VectorVariableComponentAux
    variable = vel_y
    vector_variable = velocity
    component = 'y'
  []
[]

[Variables]
  [./velocity]
    family = LAGRANGE_VEC
  [../]
  [./p]
  [../]
[]

[ICs]
  [velocity]
    type = VectorConstantIC
    x_value = 1e-15
    y_value = 1e-15
    variable = velocity
  []
[]

[Kernels]
  [./mass]
    type = INSADMass
    variable = p
  [../]
  [./mass_pspg]
    type = INSADMassPSPG
    variable = p
  [../]

  [./momentum_convection]
    type = INSADMomentumAdvection
    variable = velocity
  [../]

  [./momentum_viscous]
    type = INSADMomentumViscous
    variable = velocity
  [../]

  [./momentum_pressure]
    type = INSADMomentumPressure
    variable = velocity
    p = p
    integrate_p_by_parts = true
  [../]

  [./momentum_supg]
    type = INSADMomentumSUPG
    variable = velocity
    velocity = velocity
  [../]
[]

[BCs]
  [./no_slip]
    type = LagrangeVecFunctionDirichletBC
    variable = velocity
    boundary = 'bottom right left'
  [../]

  [./lid]
    type = LagrangeVecFunctionDirichletBC
    variable = velocity
    boundary = 'top'
    x_exact_soln = 'lid_function'
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
    prop_names = 'rho mu'
    prop_values = '1  1'
  [../]
  [ins_mat]
    type = INSADTauMaterial
    velocity = velocity
    pressure = p
    transient_term = false
    integrate_p_by_parts = true
    alpha = .1
  []
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
  type = Steady
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = 'none'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-13
  nl_max_its = 6
  l_tol = 1e-6
  l_max_its = 500
[]

[Outputs]
  exodus = true
  file_base = lid_driven_stabilized_out
  [dofmap]
    type = DOFMap
    execute_on = 'initial'
  []
[]

[Postprocessors]
  [lin]
    type = NumLinearIterations
  []
  [nl]
    type = NumNonlinearIterations
  []
  [lin_tot]
    type = CumulativeValuePostprocessor
    postprocessor = 'lin'
  []
  [nl_tot]
    type = CumulativeValuePostprocessor
    postprocessor = 'nl'
  []
[]
