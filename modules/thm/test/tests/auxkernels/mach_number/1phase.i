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
  [mach_no]
  []
  [v]
    initial_condition = 1e-3
  []
  [e]
    initial_condition = 1e5
  []
  [vel]
    initial_condition = 10.
  []
[]

[AuxKernels]
  [mach_aux]
    type = MachNumberAux
    variable = mach_no
    vel = vel
    v = v
    e = e
    fp = fp
  []
[]

[Postprocessors]
  [mach_no]
    type = ElementalVariableValue
    variable = mach_no
    elementid = 0
  []
[]

[Problem]
  solve = false
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
