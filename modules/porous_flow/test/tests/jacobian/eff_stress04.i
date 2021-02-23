# 2phase (PS)
# vanGenuchten, constant-bulk density for each phase, constant porosity, 2components (that exist in both phases)
# unsaturated
# RZ coordinate system
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Problem]
  coord_type = RZ
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [ppwater]
  []
  [sgas]
  []
[]

[AuxVariables]
  [massfrac_ph0_sp0]
  []
  [massfrac_ph1_sp0]
  []
[]

[ICs]
  [ppwater]
    type = RandomIC
    variable = ppwater
    min = 0
    max = 1
  []
  [sgas]
    type = RandomIC
    variable = sgas
    min = 0
    max = 1
  []
  [massfrac_ph0_sp0]
    type = RandomIC
    variable = massfrac_ph0_sp0
    min = 0
    max = 1
  []
  [massfrac_ph1_sp0]
    type = RandomIC
    variable = massfrac_ph1_sp0
    min = 0
    max = 1
  []
[]

[Kernels]
  [grad0]
    type = PorousFlowEffectiveStressCoupling
    biot_coefficient = 0.3
    component = 0
    variable = ppwater
  []
  [grad1]
    type = PorousFlowEffectiveStressCoupling
    biot_coefficient = 0.3
    component = 1
    variable = sgas
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'ppwater sgas'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1
    pc_max = 10
    sat_lr = 0.01
  []
[]

[Materials]
  [ppss]
    type = PorousFlow2PhasePS
    phase0_porepressure = ppwater
    phase1_saturation = sgas
    capillary_pressure = pc
  []
  [p_eff]
    type = PorousFlowEffectiveFluidPressure
  []
[]

[Preconditioning]
  [check]
    type = SMP
    full = true
    petsc_options_iname = '-snes_type'
    petsc_options_value = 'test'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 1
[]
