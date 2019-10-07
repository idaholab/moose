# Tests the Jacobian of Junction constraints. This input file is used for all 3
# equations: mass, momentum, and energy; A placeholder string is substituted
# with the name of the Junction constraint corresponding to the equation.

[JacobianTest1Phase]
  A = 2
  p = 1e5
  T = 300
  vel = 3
  snes_test_err = 1e-6
  generate_mesh = false
  use_transient_executioner = true
  fp_1phase = fp_1phase
[]

[Mesh]
  file = ../meshes/2pipes.e
  construct_side_list_from_node_list = true
[]

[FluidProperties]
  [./fp_1phase]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    cv = 1816.0
    q = -1.167e6
    p_inf = 1.0e9
    q_prime = 0
  []
[]

[Variables]
  [./junction:s]
    family = SCALAR
    order = FIRST
  [../]
[]

[ICs]
  [./s_junction_IC]
    type = ScalarConstantIC
    variable = junction:s
    value = 393.0624797
  [../]
[]

[Constraints]
  [./test_constraint]
    type = (PLACEHOLDER) # This value will be substituted using a CLI argument
    variable = rhoA
    nodes = '1 2'
    normals = '1 -1'
    K = '0.2 0.3'
    A = A
    rhoA = rhoA
    rhouA = rhouA
    rhoEA = rhoEA
    s_junction = junction:s
    vel = vel
    v = v
    e = e
    p = p
    H_junction_uo = H_junction_uo
    fp = fp_1phase
  [../]
[]

[UserObjects]
  [./H_junction_uo]
    type = JunctionStagnationEnthalpyUserObject
    boundary = '2 3'
    normals = '1 -1'
    execute_on = linear
    A = A
    rhoA = rhoA
    rhouA = rhouA
    rhoEA = rhoEA
    rho = rho
    vel = vel
    H = H
  [../]
[]
