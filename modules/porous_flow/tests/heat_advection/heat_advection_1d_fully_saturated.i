# 1phase, heat advecting with a moving fluid
# Using the FullySaturated Kernel
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 50
  xmin = 0
  xmax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [./temp]
    initial_condition = 200
  [../]
  [./pp]
  [../]
[]

[ICs]
  [./pp]
    type = FunctionIC
    variable = pp
    function = '1-x'
  [../]
[]

[BCs]
  [./pp0]
    type = PresetBC
    variable = pp
    boundary = left
    value = 1
  [../]
  [./pp1]
    type = PresetBC
    variable = pp
    boundary = right
    value = 0
  [../]
  [./spit_heat]
    type = PresetBC
    variable = temp
    boundary = left
    value = 300
  [../]
  [./suck_heat]
    type = PresetBC
    variable = temp
    boundary = right
    value = 200
  [../]
[]

[Kernels]
  [./mass_dot]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  [../]
  [./advection]
    type = PorousFlowFullySaturatedDarcyBase
    variable = pp
    gravity = '0 0 0'
  [../]
  [./energy_dot]
    type = PorousFlowEnergyTimeDerivative
    variable = temp
  [../]
  [./convection]
    type = PorousFlowFullySaturatedHeatAdvection
    variable = temp
    gravity = '0 0 0'
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
    at_nodes = true
    temperature = temp
  [../]
  [./temperature_nodal]
    type = PorousFlowTemperature
    temperature = temp
  [../]
  [./porosity]
    type = PorousFlowPorosityConst
    at_nodes = true
    porosity = 0.2
  [../]
  [./rock_heat]
    type = PorousFlowMatrixInternalEnergy
    specific_heat_capacity = 1.0
    density = 125
  [../]
  [./visc0]
    type = PorousFlowViscosityConst
    viscosity = 4.4
    phase = 0
  [../]
  [./visc_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_viscosity_qp
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1.1 0 0 0 2 0 0 0 3'
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    at_nodes = true
  [../]
  [./PS]
    type = PorousFlow1PhaseP
    at_nodes = true
    porepressure = pp
  [../]
  [./PS_qp]
    type = PorousFlow1PhaseP
    porepressure = pp
  [../]
  [./fluid_density]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 1E3
    bulk_modulus = 100.0
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    include_old = true
    at_nodes = true
    material_property = PorousFlow_fluid_phase_density_nodal
  [../]
  [./fluid_density_qp]
    type = PorousFlowDensityConstBulk
    density_P0 = 1E3
    bulk_modulus = 100.0
    phase = 0
  [../]
  [./dens_qp_all]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_density_qp
    at_nodes = false
  [../]
  [./fluid_energy]
    type = PorousFlowInternalEnergyIdeal
    at_nodes = true
    specific_heat_capacity = 2
    phase = 0
  [../]
  [./energy_all]
    type = PorousFlowJoiner
    include_old = true
    at_nodes = true
    material_property = PorousFlow_fluid_phase_internal_energy_nodal
  [../]
  [./fluid_energy_qp]
    type = PorousFlowInternalEnergyIdeal
    specific_heat_capacity = 2
    phase = 0
  [../]
  [./energy_all_qp]
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
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'gmres bjacobi 1E-15 1E-10 10000'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 0.01
  end_time = 0.6
[]

[Outputs]
  file_base = heat_advection_1d_fully_saturated
  exodus = true
  interval = 10
[]
