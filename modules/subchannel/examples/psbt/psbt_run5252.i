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
    T_in = 387.1
    P_in = 14.710
    vz_in = 0.567244
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
    total_power = 399.
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
[]

[Variables]
  [dummy_var]
  []
[]

[Kernels]
#  [dummy_kern]
#    type = Diffusion
#    variable = dummy_var
#  []
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
