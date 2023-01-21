[GlobalParams]
  gravity = '0 0 0'
  laplace = true
  integrate_p_by_parts = true
  family = LAGRANGE
  order = FIRST

  # There are multiple types of stabilization possible in incompressible
  # Navier Stokes. The user can specify supg = true to apply streamline
  # upwind petrov-galerkin stabilization to the momentum equations. This
  # is most useful for high Reynolds numbers, e.g. when inertial effects
  # dominate over viscous effects. The user can also specify pspg = true
  # to apply pressure stabilized petrov-galerkin stabilization to the mass
  # equation. PSPG is a form of Galerkin Least Squares. This stabilization
  # allows equal order interpolations to be used for pressure and velocity.
  # Finally, the alpha parameter controls the amount of stabilization.
  # For PSPG, decreasing alpha leads to increased accuracy but may induce
  # spurious oscillations in the pressure field. Some numerical experiments
  # suggest that alpha between .1 and 1 may be optimal for accuracy and
  # robustness.
  supg = true
  pspg = true
  alpha = 1e-1
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1.0
    ymin = 0
    ymax = 1.0
    nx = 64
    ny = 64
    elem_type = QUAD4
  []
  [./corner_node]
    type = ExtraNodesetGenerator
    new_boundary = 'pinned_node'
    nodes = '0'
    input = gen
  [../]
[]

[Variables]
  [./vel_x]
  [../]

  [./vel_y]
  [../]

  [./p]
  [../]
[]

[Kernels]
  # mass
  [./mass]
    type = INSMass
    variable = p
    u = vel_x
    v = vel_y
    pressure = p
  [../]

  # x-momentum, space
  [./x_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_x
    u = vel_x
    v = vel_y
    pressure = p
    component = 0
  [../]

  # y-momentum, space
  [./y_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_y
    u = vel_x
    v = vel_y
    pressure = p
    component = 1
  [../]
[]

[BCs]
  [./x_no_slip]
    type = DirichletBC
    variable = vel_x
    boundary = 'bottom right left'
    value = 0.0
  [../]

  [./lid]
    type = FunctionDirichletBC
    variable = vel_x
    boundary = 'top'
    function = 'lid_function'
  [../]

  [./y_no_slip]
    type = DirichletBC
    variable = vel_y
    boundary = 'bottom right top left'
    value = 0.0
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
    block = 0
    prop_names = 'rho mu'
    prop_values = '1  1'
  [../]
[]

[Functions]
  [./lid_function]
    # We pick a function that is exactly represented in the velocity
    # space so that the Dirichlet conditions are the same regardless
    # of the mesh spacing.
    type = ParsedFunction
    expression = '4*x*(1-x)'
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
  petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type -sub_pc_factor_levels'
  petsc_options_value = 'asm      2               ilu          4'
  line_search = 'none'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-13
  nl_max_its = 6
  l_tol = 1e-6
  l_max_its = 500
[]

[Outputs]
  exodus = true
  print_linear_converged_reason = false
  print_nonlinear_converged_reason = false
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
