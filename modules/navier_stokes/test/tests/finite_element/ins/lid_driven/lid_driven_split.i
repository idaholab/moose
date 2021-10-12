[GlobalParams]
  gravity = '0 0 0'
[]

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
    boundary = 99
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

  # x-acceleration
  [./a1]
    order = FIRST
    family = LAGRANGE

    [./InitialCondition]
      type = ConstantIC
      value = 0.0
    [../]
  [../]

  # y-acceleration
  [./a2]
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
  # split-momentum, x
  [./x_split_momentum]
    type = INSSplitMomentum
    variable = a1
    u = u
    v = v
    a1 = a1
    a2 = a2
    component = 0
  [../]

  # split-momentum, y
  [./y_split_momentum]
    type = INSSplitMomentum
    variable = a2
    u = u
    v = v
    a1 = a1
    a2 = a2
    component = 1
  [../]

  # projection-x, space
  [./x_proj_space]
    type = INSProjection
    variable = u
    a1 = a1
    a2 = a2
    pressure = p
    component = 0
  [../]

  # projection-y, space
  [./y_proj_space]
    type = INSProjection
    variable = v
    a1 = a1
    a2 = a2
    pressure = p
    component = 1
  [../]

  # projection-x, time
  [./x_proj_time]
    type = TimeDerivative
    variable = u
  [../]

  # projection-y, time
  [./y_proj_time]
    type = TimeDerivative
    variable = v
  [../]

  # Pressure
  [./pressure_poisson]
    type = INSPressurePoisson
    variable = p
    a1 = a1
    a2 = a2
  [../]
[]




[BCs]
  [./x_no_slip]
    type = DirichletBC
    variable = u
    boundary = 'bottom right left'
    value = 0.0
  [../]

  [./lid]
    type = DirichletBC
    variable = u
    boundary = 'top'
    value = 100.0
  [../]

  [./y_no_slip]
    type = DirichletBC
    variable = v
    boundary = 'bottom right top left'
    value = 0.0
  [../]

  # Acceleration boundary conditions.  What should these
  # be on the lid?  What should they be in general?  I tried pinning
  # values of acceleration at one node but that didn't seem to work.
  # I also tried setting non-zero acceleration values on the lid but
  # that didn't converge.
  [./x_no_accel]
    type = DirichletBC
    variable = a1
    boundary = 'bottom right top left'
    value = 0.0
  [../]

  [./y_no_accel]
    type = DirichletBC
    variable = a2
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
# [./FDP_Newton]
#   type = FDP
#   full = true
#   petsc_options = '-snes'
#   #petsc_options_iname = '-mat_fd_coloring_err'
#   #petsc_options_value = '1.e-10'
# [../]

[./SMP_PJFNK]
  type = SMP
  full = true

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


[../]
[]


[Executioner]
  type = Transient
  dt = 1.e-4
  dtmin = 1.e-6
  petsc_options_iname = '-ksp_gmres_restart '
  petsc_options_value = '300                '

  line_search = 'none'

  nl_rel_tol = 1e-5
  nl_max_its = 6
  l_tol = 1e-6
  l_max_its = 100
  start_time = 0.0
  num_steps = 1000
[]




[Outputs]
  file_base = lid_driven_split_out
  exodus = true
[]
