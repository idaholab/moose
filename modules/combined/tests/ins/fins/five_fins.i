[GlobalParams]
  # rho = 1000    # kg/m^3
  # mu = 0.798e-3 # Pa-s at 30C
  # cp = 4.179e3  # J/kg-K at 30C
  # k = 0.58      # W/m-K at ?C
  gravity = '0 0 0'

  # Dummy parameters
  rho = 1
  mu = 1
  cp = 1
  k = 1
[]



[Mesh]
  type = FileMesh
  file = five_fins.e
[]



[Variables]
  # x-velocity
  [./u]
    order = FIRST
    family = LAGRANGE

    [./InitialCondition]
      type = ConstantIC
      value = 0.0
    [../]
  [../]

  # y-velocity
  [./v]
    order = FIRST
    family = LAGRANGE

    [./InitialCondition]
      type = ConstantIC
      value = 0.0
    [../]
  [../]

  # x-star velocity
  [./u_star]
    order = FIRST
    family = LAGRANGE

    [./InitialCondition]
      type = ConstantIC
      value = 0.0
    [../]
  [../]

  # y-star velocity
  [./v_star]
    order = FIRST
    family = LAGRANGE

    [./InitialCondition]
      type = ConstantIC
      value = 0.0
    [../]
  [../]

  # Pressure
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
  [./div_u]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./div_u_star]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]



[Functions]
  # Note1: if you change the height of the domain or translate it
  # in the y-direction, you'll need to update this function (both the
  # scale factor and the endpoint values).
  #
  # Note2: the leading coefficient sets the maximum flow velocity, the
  # second coefficient normalizes the parabola to a max value of 1.0, and
  # the whole thing is scaled by the hyperbolic tangent of t so that
  # the flow ramps slowly over time.  You will reach 99% of the desired
  # velocity around t=2.65, i.e. tanh(2.65) ~ 0.99.
  [./parabola]
    type = ParsedFunction
    value = 10*(-4.0)*(y-0.5)*(y+0.5)*tanh(t)
  [../]
[]


[Kernels]
  [./x_chorin_predictor]
    type = INSChorinPredictor
    variable = u_star
    u = u
    v = v
    component = 0
    chorin_implicit = true
  [../]

  [./y_chorin_predictor]
    type = INSChorinPredictor
    variable = v_star
    u = u
    v = v
    component = 1
    chorin_implicit = true
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
  [./div_u_aux]
    type = INSDivergenceAux
    variable = div_u
    u = u
    v = v
  [../]
[]


[AuxKernels]
  [./div_u_star_aux]
    type = INSDivergenceAux
    variable = div_u_star
    u = u_star
    v = v_star
  [../]
[]






[BCs]
  [./u_no_slip]
    type = DirichletBC
    variable = u
    boundary = 'bottom top fins fins_vertical'
    value = 0.0
  [../]

  [./u_inlet]
    type = FunctionDirichletBC
    variable = u
    boundary = 'left'
    function = parabola
  [../]

  [./v_no_slip]
    type = DirichletBC
    variable = v
    boundary = 'left bottom top fins fins_vertical'
    value = 0.0
  [../]

  # Make u_star, v_star satsify all the same BCs as the real velocity...
  [./u_star_no_slip]
    type = DirichletBC
    variable = u_star
    boundary = 'bottom top fins fins_vertical'
    value = 0.0
  [../]

  [./u_star_inlet]
    type = FunctionDirichletBC
    variable = u_star
    boundary = 'left'
    function = parabola
  [../]

  [./v_star_no_slip]
    type = DirichletBC
    variable = v_star
    boundary = 'left bottom top fins fins_vertical'
    value = 0.0
  [../]

  # dp/dn=0 on solid walls in the classical Chorin method.
  # We seem to get better results using dp/dn "implicit" on solid walls.
  [./pressure_implicit]
    type = ImplicitNeumannBC
    variable = p
    boundary = 'left bottom top fins fins_vertical'
  [../]


  # On the outlet, A Dirichlet value for pressure is specified.
  [./pressure_outlet]
    type = DirichletBC
    variable = p
    boundary = 'right'
    value = 0
  [../]
[]



[Preconditioning]
#active = 'FDP_Newton'
#active = 'SMP_PJFNK'
active = 'SMP_Newton'

[./FDP_Newton]
type = FDP
full = true
petsc_options = '-snes'
#petsc_options_iname = '-mat_fd_coloring_err'
#petsc_options_value = '1.e-10'
[../]

# For some reason, nonlinear convergence with JFNK is poor, but it
# seems to be OK for SMP_Newton.  This may indicate a a scaling issue
# in the JFNK case....
[./SMP_PJFNK]
  type = SMP
  full = true
  petsc_options = '-snes_mf_operator'
[../]

[./SMP_Newton]
  type = SMP
  full = true
  petsc_options = '-snes'
[../]
[]


[Executioner]
  type = Transient
  dt = 1.e-2
  dtmin = 1.e-6
  perf_log = true
  petsc_options_iname = '-ksp_gmres_restart -snes_linesearch_type'
  petsc_options_value = '300                basic'
  nl_rel_tol = 1e-5
  nl_max_its = 6
  l_tol = 1e-6
  l_max_its = 300
  start_time = 0.0
  num_steps = 40
[]




[Output]
  file_base = five_fins_out
  interval = 1
  output_initial = true
  exodus = true
[]
