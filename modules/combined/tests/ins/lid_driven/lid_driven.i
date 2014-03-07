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
  nx = 160
  ny = 160
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

 # Temperature
 [./T]
   order = SECOND
   family = LAGRANGE

   [./InitialCondition]
     type = ConstantIC
     value = 1.0
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



 # temperature
 [./temperature_time]
   type = INSTemperatureTimeDerivative
   variable = T
 [../]

 [./temperature_space]
   type = INSTemperature
   variable = T
   u = u
   v = v
   p = p
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

 [./T_hot]
   type = DirichletBC
   variable = T
   #boundary = '0'
   boundary = 'bottom'
   value = 1
 [../]

 [./T_cold]
   type = DirichletBC
   variable = T
   #boundary = '2'
   boundary = 'top'
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

# FSP options to recursive split (u,v,p,T) -> ((u,v,p), T) -> (((u,v), p), T)
# 0 = (u,v,p), 00 = (u,v), 01 = (p)
# 1 = (T)
# -pc_type fieldsplit -dm_moose_nfieldsplits 2 -dm_moose_fieldsplit_0_vars u,v,p -dm_moose_fieldsplit_1_vars T -pc_fieldsplit_type additive -ksp_monitor -fieldsplit_0_pc_type fieldsplit -fieldsplit_0_dm_moose_nfieldsplits 2 -fieldsplit_0_dm_moose_fieldsplit_0_vars u,v -fieldsplit_0_dm_moose_fieldsplit_1_vars p -fieldsplit_0_pc_fieldsplit_type schur -fieldsplit_0_ksp_monitor -fieldsplit_0_fieldsplit_0_pc_type lu


# Options to use "LSC" for the Schur complement solve:
# -pc_type fieldsplit -dm_moose_nfieldsplits 2 -dm_moose_fieldsplit_0_vars u,v,p -dm_moose_fieldsplit_1_vars T -pc_fieldsplit_type additive -fieldsplit_0_pc_type fieldsplit -fieldsplit_0_dm_moose_nfieldsplits 2 -fieldsplit_0_dm_moose_fieldsplit_0_vars u,v -fieldsplit_0_dm_moose_fieldsplit_1_vars p -fieldsplit_0_pc_fieldsplit_type schur -fieldsplit_0_fieldsplit_0_pc_type lu -fieldsplit_0_pc_schur_precondition self -fieldsplit_0_fieldsplit_1_pc_type lsc
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
  dtmin = 1.e-2
  perf_log = true

  # Basic GMRES/linesearch options only
  petsc_options_iname = '-ksp_gmres_restart '
  petsc_options_value = '300                '

  line_search = 'none'


  # MOOSE does not correctly read these options!! Always run with actual command line arguments if
  # you want to guarantee you are getting pilut!  Also, even though there are pilut defaults,
  # i'm not sure that petsc actually passes them through, so always specify them!.
  #
  # PILUT options:
  # -pc_type hypre -pc_hypre_type pilut -pc_hypre_pilut_factorrowsize 20 -pc_hypre_pilut_tol 1.e-4
  #
  # PETSc ILU options (parallel):
  # -sub_pc_type ilu -sub_pc_factor_levels 2
  #
  # ASM options (to be used in conjunction with ILU sub_pc)
  # -pc_type asm -pc_asm_overlap 2

  nl_rel_tol = 1e-5
  nl_max_its = 6
  l_tol = 1e-6
  l_max_its = 500
  start_time = 0.0
  num_steps = 40
[]




[Output]
  file_base = lid_driven_out
  interval = 1
  output_initial = true
  exodus = true
[]
