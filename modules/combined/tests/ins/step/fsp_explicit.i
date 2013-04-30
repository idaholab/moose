[GlobalParams]
  # kg/m^3
  # Pa-s at 30C
  # J/kg-K at 30C
  # W/m-K at ?C
  # Dummy parameters
  gravity = '0 0 0'
  rho = 1
  mu = 1
  cp = 1
  k = 1
[]

[Mesh]
  type = FileMesh
  file = step.e
  uniform_refine = 3
[]

[Variables]
  # x-velocity
  # y-velocity
  # x-star velocity
  # y-star velocity
  # Pressure
  [./u]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = ConstantIC
      value = 0.0
    [../]
  [../]
  [./v]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = ConstantIC
      value = 0.0
    [../]
  [../]
  [./u_star]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = ConstantIC
      value = 0.0
    [../]
  [../]
  [./v_star]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = ConstantIC
      value = 0.0
    [../]
  [../]
  [./p]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = ConstantIC
      value = 0
    [../]
  [../]
[]

[AuxVariables]
  [./vel_mag]
  [../]
[]

[Functions]
  # Note: if you change the height of the domain or translate it
  # in the y-direction, you'll need to update this function!
  # Note2: the leading coefficient sets the maximum flow velocity, the
  # second coefficient normalizes the parabola to a max value of 1.0, and
  # the whole thing is scaled by the hyperbolic tangent of t so that
  # the flow ramps slowly over time.  You will reach 99% of the desired
  # velocity around t=2.65, i.e. tanh(2.65) ~ 0.99.
  [./parabola]
    type = ParsedFunction
    value = 100*(-4.0)*(y-0.5)*(y+0.5)*tanh(t)
  [../]
[]

[Kernels]
  [./x_chorin_predictor]
    type = INSChorinPredictor
    variable = u_star
    u = u
    v = v
    component = 0
    chorin_implicit = false
  [../]
  [./y_chorin_predictor]
    type = INSChorinPredictor
    variable = v_star
    u = u
    v = v
    component = 1
    chorin_implicit = false
  [../]
  [./x_chorin_corrector]
    type = INSChorinCorrector
    variable = u
    u_star = u_star
    v_star = v_star
    p = p
    component = 0
  [../]
  [./y_chorin_corrector]
    type = INSChorinCorrector
    variable = v
    u_star = u_star
    v_star = v_star
    p = p
    component = 1
  [../]
  [./chorin_pressure_poisson]
    type = INSChorinPressurePoisson
    variable = p
    u_star = u_star
    v_star = v_star
  [../]
[]

[AuxKernels]
  [./vel_mag]
    type = VectorMagnitudeAux
    variable = vel_mag
    x = u
    y = v
  [../]
[]

[BCs]
  # Make u_star, v_star satsify all the same BCs as the real velocity...
  # dp/dn=0 on solid walls.
  # On the outlet, A Dirichlet value for pressure is specified.
  [./u_no_slip]
    type = DirichletBC
    variable = u
    boundary = 'bottom top'
    value = 0.0
  [../]
  [./u_inlet]
    type = FunctionDirichletBC
    variable = u
    boundary = left
    function = parabola
  [../]
  [./v_no_slip]
    type = DirichletBC
    variable = v
    boundary = 'left bottom top'
    value = 0.0
  [../]
  [./u_star_no_slip]
    type = DirichletBC
    variable = u_star
    boundary = 'bottom top'
    value = 0.0
  [../]
  [./u_star_inlet]
    type = FunctionDirichletBC
    variable = u_star
    boundary = left
    function = parabola
  [../]
  [./v_star_no_slip]
    type = DirichletBC
    variable = v_star
    boundary = 'left bottom top'
    value = 0.0
  [../]
  [./pressure_outlet]
    type = DirichletBC
    variable = p
    boundary = right
    value = 0
  [../]
[]

[Postprocessors]
  [./dt]
    type = INSExplicitTimestepSelector
    beta = 0.25
    vel_mag = vel_mag
  [../]
[]

[FieldSplits]
  [./velocity]
    name = velocity
    vars = 'u v'
    petsc_options_iname = '-pc_type -pc_asm_overlap'
    petsc_options_value = 'asm 2'
  [../]
  [./star]
    name = star
    vars = 'u_star v_star'
    petsc_options_iname = '-pc_type -sub_pc_type'
    petsc_options_value = 'asm jacobi'
  [../]
  [./pressure]
    name = pressure
    vars = p
    petsc_options_iname = '-pc_type -pc_hypre_type'
    petsc_options_value = '   hypre boomeramg'
  [../]
[]

[Preconditioning]
  # active = 'FDP_Newton'
  # active = 'SMP_PJFNK'
  # For some reason, nonlinear convergence with JFNK is poor, but it
  # seems to be OK for SMP_Newton.  This may indicate a a scaling issue
  # in the JFNK case....
  # active = 'SMP_Newton'
  active = 'FSP_solve'
  [./FDP_Newton]
    # petsc_options_iname = '-mat_fd_coloring_err'
    # petsc_options_value = '1.e-10'
    type = FDP
    full = true
    petsc_options = -snes
  [../]
  [./SMP_PJFNK]
    type = SMP
    full = true
    petsc_options = -snes_mf_operator
  [../]
  [./SMP_Newton]
    type = SMP
    full = true
    petsc_options = -snes
  [../]
  [./PBP]
    type = PBP
    solve_order = 'u v u_star v_star p'
    preconditioner = 'ASM ASM ASM ASM AMG'
    petsc_options = -snes_mf_operator
  [../]
  [./FSP_solve]
    type = FSP
    full = true
    off_diag_row = 'u v u_star u_star v_star v_star p      p'
    off_diag_column = 'p p u      v      u      v      u_star v_star'
    splits = 'star pressure velocity'
    fieldsplit_type = multiplicative
    petsc_options = -snes
  [../]
  [./SMP_Newton_hypre]
    type = SMP
    full = true
    petsc_options_value = 'hypre boomeramg'
    petsc_options = -snes
    petsc_options_iname = '-pc_type -pc_hypre_type'
  [../]
[]

[Executioner]
  type = INSExplicitExecutioner
  dt = 2e-3
  dtmin = 1.e-6
  perf_log = true
  petsc_options = -ksp_monitor
  petsc_options_iname = '-ksp_gmres_restart -snes_linesearch_type'
  petsc_options_value = '300                basic'
  nl_rel_tol = 1e-5
  nl_max_its = 6
  l_tol = 1e-6
  l_max_its = 1000
  start_time = 0.0
  num_steps = 10000
[]

[Output]
  file_base = fsp_explicit_out
  output_initial = true
  exodus = true
  perf_log = true
[]

