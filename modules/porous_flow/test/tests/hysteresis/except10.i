# Exception testing: S_gr_max too large
[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    number_fluid_phases = 1
    number_fluid_components = 1
    porous_flow_vars = pp
  []
[]

[Variables]
  [pp]
  []
[]

[Kernels]
  [dummy]
    type = Diffusion
    variable = pp
  []
[]

[Materials]
  [hys_order]
    type = PorousFlowHysteresisOrder
  []
  [saturation_calculator]
    type = PorousFlow1PhaseHysP
    alpha_d = 10.0
    alpha_w = 10.0
    n_d = 1.9
    n_w = 1.9
    S_l_min = 0.1
    S_lr = 0.2
    S_gr_max = 0.9
    Pc_max = 3.0
    porepressure = pp
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 1
[]

[Outputs]
  csv = true
[]
