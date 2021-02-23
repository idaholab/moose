# 2phase (PP)
# vanGenuchten, constant-bulk density for each phase, constant porosity, 2components (that exist in both phases)
# unsaturated
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [ppwater]
  []
  [ppgas]
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
    min = -1
    max = 0
  []
  [ppgas]
    type = RandomIC
    variable = ppgas
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
    variable = ppgas
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'ppwater ppgas'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1
  []
[]

[Materials]
  [ppss]
    type = PorousFlow2PhasePP
    phase0_porepressure = ppwater
    phase1_porepressure = ppgas
    capillary_pressure = pc
  []
  [p_eff]
    type = PorousFlowEffectiveFluidPressure
  []
[]

[Preconditioning]
  active = check
  [andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000'
  []
  [check]
    type = SMP
    full = true
    petsc_options = '-snes_test_display'
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 test'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 1
[]

[Outputs]
  exodus = false
[]
