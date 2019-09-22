################################################################################
# Inputs relevant to the subchannel solver
################################################################################

[Mesh]
  type = SubchannelMesh
  nx = 6
  ny = 6
  max_dz = 0.01
[]

[AuxVariables]
  [vz]
  []
  [P]
  []
  [h]
  []
  [T]
  []
  [rho]
  []
  [A]
  []
  [w_perim]
  []
  [q_prime]
  []
[]

[UserObjects]
  [subchannel_solver]
    type = SubchannelSolver
    execute_on = "INITIAL"
    vz = vz
    P = P
    h = h
    T = T
    rho = rho
    flow_area = A
    wetted_perimeter = w_perim
    q_prime = q_prime
    T_in = 524.6
    P_in = 16.583
    vz_in = 0.71971359
  []
[]

[ICs]
  [A_IC]
    type = PsbtFlowAreaIC
    variable = A
  []
  [w_perim_IC]
    type = PsbtWettedPerimIC
    variable = w_perim
  []
  [q_prime_IC]
    type = PsbtPowerIC
    variable = q_prime
    total_power = 413.
  []
[]

[Outputs]
  exodus = true
[]

################################################################################
# Stuff needed to make the program execute
################################################################################

[Executioner]
  type = Steady
  nl_rel_tol = 0.9
  l_tol = 0.9
[]

[Variables]
  [dummy_var]
  []
[]

[Kernels]
  [dummy_kern]
    type = Diffusion
    variable = dummy_var
  []
[]

[BCs]
  [dummy_bc1]
    variable = dummy_var
    boundary = 'bottom'
    type = DirichletBC
    value = 0
  []
  [dummy_bc2]
    variable = dummy_var
    boundary = 'top'
    type = DirichletBC
    value = 1
  []
[]

################################################################################
# A multiapp that projects data to a detailed mesh
################################################################################

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    input_files = "sub.i"
    execute_on = "NONLINEAR"
  []
[]

[Transfers]
  [xfer_vz]
    type = MultiAppNearestNodeTransfer
    multi_app = sub
    direction = to_multiapp
    source_variable = vz
    variable = vz
  []
  [xfer_P]
    type = MultiAppNearestNodeTransfer
    multi_app = sub
    direction = to_multiapp
    source_variable = P
    variable = P
  []
  [xfer_h]
    type = MultiAppNearestNodeTransfer
    multi_app = sub
    direction = to_multiapp
    source_variable = h
    variable = h
  []
  [xfer_T]
    type = MultiAppNearestNodeTransfer
    multi_app = sub
    direction = to_multiapp
    source_variable = T
    variable = T
  []
  [xfer_rho]
    type = MultiAppNearestNodeTransfer
    multi_app = sub
    direction = to_multiapp
    source_variable = rho
    variable = rho
  []
[]
