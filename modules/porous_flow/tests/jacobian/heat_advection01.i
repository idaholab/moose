# 1phase, unsaturated, heat advection
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
    type = PorousFlowHeatAdvection
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
  [./nnn]
    type = PorousFlowNodeNumber
    on_initial_only = true
  [../]
  [./visc0]
    type = PorousFlowViscosityConst
    viscosity = 1
    phase = 0
  [../]
  [./visc_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_viscosity
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1 0 0 0 2 0 0 0 3'
  [../]
  [./relperm]
    type = PorousFlowRelativePermeabilityCorey
    n_j = 2
    phase = 0
  [../]
  [./relperm_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_relative_permeability
  [../]
  [./PS]
    type = PorousFlow1PhaseP_VG
    porepressure = pp
    al = 1.3
    m = 0.6
  [../]
  [./fluid_density]
    type = PorousFlowDensityConstBulk
    density_P0 = 1.1
    bulk_modulus = 0.5
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_density
  [../]
  [./dens_qp_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_density_qp
    at_qps = true
  [../]
  [./fluid_energy]
    type = PorousFlowInternalEnergyIdeal
    specific_heat_capacity = 1.1
    phase = 0
  [../]
  [./energy_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_internal_energy_nodal
  [../]
  [./fluid_enthalpy]
    type = PorousFlowEnthalpy
    phase = 0
  [../]
  [./enthalpy_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_enthalpy_nodal
  [../]
[]

[Preconditioning]
  active = check
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000'
  [../]
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
