# Use ReynoldsNumberAux to compute Reynolds number

[GlobalParams]
  family = MONOMIAL
  order = CONSTANT
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  allow_renumbering = false
[]

[FluidProperties]
  [fp]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    q = -1167e3
    q_prime = 0
    p_inf = 1.e9
    cv = 1816
  []
[]

[AuxVariables]
  [reynolds_no]
  []
  [rho]
    initial_condition = 1000
  []
  [vel]
    initial_condition = 1
  []
  [D_h]
    initial_condition = 1.1283791671e-02
  []
  [v]
    initial_condition = 1e-3
  []
  [e]
    initial_condition = 1e5
  []
[]

[AuxKernels]
  [rn_aux]
    type = ReynoldsNumberAux
    variable = reynolds_no
    rho = rho
    vel = vel
    D_h = D_h
    v = v
    e = e
    fp = fp
  []
[]

[Problem]
  solve = false
[]

[Postprocessors]
  [reynolds_no]
    type = ElementalVariableValue
    variable = reynolds_no
    elementid = 0
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 1
  abort_on_solve_fail = true
[]

[Outputs]
  csv = true
  execute_on = TIMESTEP_END
[]
