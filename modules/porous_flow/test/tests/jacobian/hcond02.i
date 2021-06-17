# 2phase heat conduction
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  xmin = 0
  xmax = 1
  ny = 1
  ymin = 0
  ymax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [pgas]
  []
  [pwater]
  []
  [temp]
  []
[]

[ICs]
  [pgas]
    type = RandomIC
    variable = pgas
    max = 1.0
    min = 0.0
  []
  [pwater]
    type = RandomIC
    variable = pwater
    max = 0.0
    min = -1.0
  []
  [temp]
    type = RandomIC
    variable = temp
    max = 1.0
    min = 0.0
  []
[]


[Kernels]
  [dummy_pgas]
    type = Diffusion
    variable = pgas
  []
  [dummy_pwater]
    type = Diffusion
    variable = pwater
  []
  [heat_conduction]
    type = PorousFlowHeatConduction
    variable = temp
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pgas temp pwater'
    number_fluid_phases = 2
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = temp
  []
  [thermal_conductivity]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '1.1 0.1 0.3 0.1 2.2 0 0.3 0 3.3'
    wet_thermal_conductivity = '2.1 0.1 0.3 0.1 1.2 0 0.3 0 1.1'
    exponent = 1.7
    aqueous_phase_number = 1
  []
  [ppss]
    type = PorousFlow2PhasePP
    phase0_porepressure = pwater
    phase1_porepressure = pgas
    capillary_pressure = pc
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
