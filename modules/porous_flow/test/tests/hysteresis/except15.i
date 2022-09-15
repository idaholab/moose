# Exception: attempting to use PorousFlow2PhaseHysPS in a 1-phase situation
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
    porous_flow_vars = 'pp'
  []
[]

[Variables]
  [pp]
    initial_condition = 0
  []
[]

[Kernels]
  [mass_conservation]
    type = PorousFlowMassTimeDerivative
    variable = pp
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
  []
[]

[Materials]
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [temperature]
    type = PorousFlowTemperature
  []
  [massfrac]
    type = PorousFlowMassFraction
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [hys_order_material]
    type = PorousFlowHysteresisOrder
  []
  [pc_calculator]
    type = PorousFlow2PhaseHysPS
    alpha_d = 10.0
    alpha_w = 7.0
    n_d = 1.5
    n_w = 1.9
    S_l_min = 0.1
    S_lr = 0.2
    S_gr_max = 0.3
    Pc_max = 12.0
    high_ratio = 0.9
    low_extension_type = quadratic
    high_extension_type = power
    phase0_porepressure = pp
    phase1_saturation = pp
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 0.5
  end_time = 19
  nl_abs_tol = 1E-10
[]

[Outputs]
  csv = true
[]
