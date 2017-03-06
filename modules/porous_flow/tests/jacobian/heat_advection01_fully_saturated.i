# 1phase, using fully-saturated version, heat advection
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
  [./temp]
  [../]
  [./pp]
  [../]
[]

[ICs]
  [./temp]
    type = RandomIC
    variable = temp
    max = 1.0
    min = 0.0
  [../]
  [./pp]
    type = RandomIC
    variable = pp
    max = 0.0
    min = -1.0
  [../]
[]


[Kernels]
  [./pp]
    type = TimeDerivative
    variable = pp
  [../]
  [./heat_advection]
    type = PorousFlowFullySaturatedHeatAdvection
    variable = temp
    gravity = '1 2 3'
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'temp pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
    temperature = temp
  [../]
  [./visc0]
    type = PorousFlowViscosityConst
    viscosity = 1
    phase = 0
  [../]
  [./visc_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_viscosity_qp
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    at_nodes = false
    permeability = '1 0 0 0 2 0 0 0 3'
  [../]
  [./PS]
    type = PorousFlow1PhaseP
    porepressure = pp
  [../]
  [./fluid_density]
    type = PorousFlowDensityConstBulk
    density_P0 = 1.1
    bulk_modulus = 0.5
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_density_qp
  [../]
  [./fluid_energy_nodal]
    type = PorousFlowInternalEnergyIdeal
    specific_heat_capacity = 1.1
    phase = 0
  [../]
  [./energy_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_internal_energy_qp
  [../]
  [./fluid_enthalpy]
    type = PorousFlowEnthalpy
    phase = 0
  [../]
  [./enthalpy_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_enthalpy_qp
  [../]
[]

[Preconditioning]
  active = check
  [./check]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 test'
  [../]
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
