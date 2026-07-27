# 0phase heat conduction, AD
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
  [temp]
  []
[]

[ICs]
  [temp]
    type = RandomIC
    variable = temp
    max = 1.0
    min = 0.0
  []
[]

[Kernels]
  [heat_conduction]
    type = ADPorousFlowHeatConduction
    variable = temp
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'temp'
    number_fluid_phases = 0
    number_fluid_components = 0
  []
[]

[Materials]
  [temperature]
    type = ADPorousFlowTemperature
    temperature = temp
  []
  [thermal_conductivity]
    type = ADPorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '1.1 0.1 0.3 0.1 2.2 0 0.3 0 3.3'
  []
[]

[Preconditioning]
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
