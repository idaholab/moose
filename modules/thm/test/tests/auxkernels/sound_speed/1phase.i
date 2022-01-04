# Use SoundSpeedAux to compute sound speed.

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
  [sound_speed]
  []
  [e]
    initial_condition = 1e5
  []
  [v]
    initial_condition = 1e-3
  []
[]

[AuxKernels]
  [sound_speed_aux]
    type = SoundSpeedAux
    variable = sound_speed
    e = e
    v = v
    fp = fp
  []
[]

[Problem]
  solve = false
[]

[Postprocessors]
  [c]
    type = ElementalVariableValue
    variable = sound_speed
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
