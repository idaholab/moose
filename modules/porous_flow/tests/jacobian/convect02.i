# 2phase, unsaturated, convective flux
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
  [./pgas]
  [../]
  [./pwater]
  [../]
[]

[ICs]
  [./pgas]
    type = RandomIC
    variable = pgas
    max = 1.0
    min = 0.0
  [../]
  [./pwater]
    type = RandomIC
    variable = pwater
    max = 0.0
    min = -1.0
  [../]
  [./temp]
    type = RandomIC
    variable = temp
    max = 1.0
    min = 0.0
  [../]
[]


[Kernels]
  [./dummy_pgas]
    type = Diffusion
    variable = pgas
  [../]
  [./dummy_pwater]
    type = Diffusion
    variable = pwater
  [../]
  [./convection]
    type = PorousFlowConvectiveFlux
    variable = temp
    gravity = '1 2 3'
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'temp pgas pwater'
    number_fluid_phases = 2
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
  [./visc1]
    type = PorousFlowViscosityConst
    viscosity = 1.3
    phase = 1
  [../]
  [./visc_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_viscosity
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1 0 0 0 2 0 0 0 3'
  [../]
  [./relperm0]
    type = PorousFlowRelativePermeabilityCorey
    n_j = 2
    phase = 0
  [../]
  [./relperm1]
    type = PorousFlowRelativePermeabilityCorey
    n_j = 3
    phase = 1
  [../]
  [./relperm_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_relative_permeability
  [../]
  [./ppss]
    type = PorousFlow2PhasePP_VG
    phase0_porepressure = pwater
    phase1_porepressure = pgas
    m = 0.5
    al = 1
  [../]
  [./fluid_density0]
    type = PorousFlowDensityConstBulk
    density_P0 = 1.1
    bulk_modulus = 0.5
    phase = 0
  [../]
  [./fluid_density1]
    type = PorousFlowDensityConstBulk
    density_P0 = 0.7
    bulk_modulus = 0.8
    phase = 1
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
  [./fluid_energy0]
    type = PorousFlowInternalEnergyIdeal
    specific_heat_capacity = 1.1
    phase = 0
  [../]
  [./fluid_energy1]
    type = PorousFlowInternalEnergyIdeal
    specific_heat_capacity = 1.6
    phase = 1
  [../]
  [./energy_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_internal_energy_nodal
  [../]
  [./fluid_enthalpy0]
    type = PorousFlowEnthalpy
    phase = 0
  [../]
  [./fluid_enthalpy1]
    type = PorousFlowEnthalpy
    phase = 1
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
