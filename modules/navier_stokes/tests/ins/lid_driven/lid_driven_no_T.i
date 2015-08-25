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
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1.0
  ymin = 0
  ymax = 1.0
  nx = 20
  ny = 20
  elem_type = QUAD9
[]

[MeshModifiers]
  [./corner_node]
    type = AddExtraNodeset
    boundary = 99
    nodes = '0'
  [../]
[]

[Variables]
  # x-velocity
  [./u]
    order = SECOND
    family = LAGRANGE

    [./InitialCondition]
      type = ConstantIC
      value = 0.0
    [../]
  [../]

  # y-velocity
  [./v]
    order = SECOND
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
      value = 0 # This number is arbitrary for NS...
    [../]
  [../]
[]



[Kernels]
  # mass
  [./mass]
    type = INSMass
    variable = p
    u = u
    v = v
    p = p
  [../]



  # x-momentum, time
  [./x_momentum_time]
    type = INSMomentumTimeDerivative
    variable = u
  [../]

  # x-momentum, space
  [./x_momentum_space]
    type = INSMomentum
    variable = u
    u = u
    v = v
    p = p
    component = 0
  [../]



  # y-momentum, time
  [./y_momentum_time]
    type = INSMomentumTimeDerivative
    variable = v
  [../]

  # y-momentum, space
  [./y_momentum_space]
    type = INSMomentum
    variable = v
    u = u
    v = v
    p = p
    component = 1
  [../]
[]




[BCs]
  [./x_no_slip]
    type = DirichletBC
    variable = u
    # boundary = '0 1 3'
    boundary = 'bottom right left'
    value = 0.0
  [../]

  [./lid]
    type = DirichletBC
    variable = u
    # boundary = '2'
    boundary = 'top'
    value = 100.0
  [../]

  [./y_no_slip]
    type = DirichletBC
    variable = v
    # boundary = '0 1 2 3'
    boundary = 'bottom right top left'
    value = 0.0
  [../]

  [./pressure_pin]
    type = DirichletBC
    variable = p
    boundary = '99'
    value = 0
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

# Run FSP with the following options:
# '-snes_monitor -ksp_monitor -dm_moose_nfieldsplits 2 -dm_moose_fieldsplit_0_vars u,v -dm_moose_fieldsplit_1_vars p  -pc_type fieldsplit -pc_fieldsplit_type schur'
# [./FSP]
#   type = FSP
#   full = true
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
  dt = 1.e-2
  dtmin = 1.e-6
  perf_log = true
  petsc_options_iname = '-ksp_gmres_restart '
  petsc_options_value = '300                '

  line_search = 'none'

  nl_rel_tol = 1e-5
  nl_max_its = 6
  l_tol = 1e-6
  l_max_its = 100
  start_time = 0.0
  num_steps = 40
[]




[Outputs]
  file_base = lid_driven_no_T_out
  exodus = true
[]
