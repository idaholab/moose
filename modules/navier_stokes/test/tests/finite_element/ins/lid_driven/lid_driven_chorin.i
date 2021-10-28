[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1.0
    ymin = 0
    ymax = 1.0
    nx = 40
    ny = 40
    elem_type = QUAD4
  []
  [./corner_node]
    type = ExtraNodesetGenerator
    new_boundary = 99
    nodes = '0'
    input = gen
  [../]
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



[Kernels]
  [./x_chorin_predictor]
    type = INSChorinPredictor
    variable = u_star
    u = u
    v = v
    u_star = u_star
    v_star = v_star
    component = 0
    predictor_type = 'new'
  [../]

  [./y_chorin_predictor]
    type = INSChorinPredictor
    variable = v_star
    u = u
    v = v
    u_star = u_star
    v_star = v_star
    component = 1
    predictor_type = 'new'
  [../]

  [./x_chorin_corrector]
    type = INSChorinCorrector
    variable = u
    u_star = u_star
    v_star = v_star
    pressure = p
    component = 0
  [../]

  [./y_chorin_corrector]
    type = INSChorinCorrector
    variable = v
    u_star = u_star
    v_star = v_star
    pressure = p
    component = 1
  [../]

  [./chorin_pressure_poisson]
    type = INSChorinPressurePoisson
    variable = p
    u_star = u_star
    v_star = v_star
  [../]
[]

[BCs]
  [./u_no_slip]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 'bottom right left'
    value = 0.0
  [../]

  [./u_lid]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 'top'
    value = 100.0
  [../]

  [./v_no_slip]
    type = DirichletBC
    variable = v
    preset = false
    boundary = 'bottom right top left'
    value = 0.0
  [../]

  # Make u_star satsify all the same variables as the real velocity.
  [./u_star_no_slip]
    type = DirichletBC
    variable = u_star
    preset = false
    boundary = 'bottom right left'
    value = 0.0
  [../]

  [./u_star_lid]
    type = DirichletBC
    variable = u_star
    preset = false
    boundary = 'top'
    value = 100.0
  [../]

  [./v_star_no_slip]
    type = DirichletBC
    variable = v_star
    preset = false
    boundary = 'bottom right top left'
    value = 0.0
  [../]

  # With solid walls everywhere, we specify dp/dn=0, i.e the
  # "natural BC" for pressure.  Technically the problem still
  # solves without pinning the pressure somewhere, but the pressure
  # bounces around a lot during the solve, possibly because of
  # the addition of arbitrary constants.
  [./pressure_pin]
    type = DirichletBC
    variable = p
    preset = false
    boundary = '99'
    value = 0
  [../]
[]

[Materials]
  [./const]
    type = GenericConstantMaterial
    block = 0
    # rho = 1000    # kg/m^3
    # mu = 0.798e-3 # Pa-s at 30C
    # cp = 4.179e3  # J/kg-K at 30C
    # k = 0.58      # W/m-K at ?C

    # Dummy parameters
    prop_names = 'rho mu cp k'
    prop_values = '1  1  1  1'
  [../]
[]

[Preconditioning]
#active = 'FDP_Newton'
#active = 'SMP_PJFNK'
active = 'SMP_Newton'

[./FDP_Newton]
type = FDP
full = true

solve_type = 'NEWTON'

#petsc_options_iname = '-mat_fd_coloring_err'
#petsc_options_value = '1.e-10'
[../]

# For some reason, nonlinear convergence with JFNK is poor, but it
# seems to be OK for SMP_Newton.  This may indicate a a scaling issue
# in the JFNK case....
[./SMP_PJFNK]
  type = SMP
  full = true

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


[../]

[./SMP_Newton]
  type = SMP
  full = true

  solve_type = 'NEWTON'

[../]
[]


[Executioner]
  type = Transient
  # Note: the explicit case with lid velocity = 100 and a 40x40 was unstable
  # for dt=1.e-4, even though the restriction should be dt < dx/|u| = 1/4000 = 2.5e-4
  #
  dt = 1.e-3
  dtmin = 1.e-6
  petsc_options_iname = '-ksp_gmres_restart '
  petsc_options_value = '300                '

  line_search = 'none'

  nl_rel_tol = 1e-12
  nl_max_its = 6
  l_max_its = 300
  start_time = 0.0
  num_steps = 5

  automatic_scaling = true
  verbose = true
  compute_scaling_once = false
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  file_base = lid_driven_chorin_out
  exodus = true
[]
