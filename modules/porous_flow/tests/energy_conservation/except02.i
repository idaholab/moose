# checking that the heat-energy postprocessor throws the correct error if the kernel_variable_number is illegal
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 3
  xmin = 0
  xmax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [./pp]
  [../]
  [./temp]
  [../]
[]

[ICs]
  [./tinit]
    type = FunctionIC
    function = '100*x'
    variable = temp
  [../]
  [./pinit]
    type = FunctionIC
    function = x
    variable = pp
  [../]
[]

[Kernels]
  [./dummyt]
    type = TimeDerivative
    variable = temp
  [../]
  [./dummyp]
    type = TimeDerivative
    variable = pp
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
  [./porosity]
    type = PorousFlowPorosityConst
    at_nodes = true
    porosity = 0.1
  [../]
  [./rock_heat]
    type = PorousFlowMatrixInternalEnergy
    at_nodes = true
    specific_heat_capacity = 2.2
    density = 0.5
  [../]
  [./ppss]
    type = PorousFlow1PhaseP_VG
    at_nodes = true
    porepressure = pp
    al = 1
    m = 0.5
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    at_nodes = true
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 1
    bulk_modulus = 1
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    at_nodes = true
    include_old = true
    material_property = PorousFlow_fluid_phase_density_nodal
  [../]
  [./water_heat]
    type = PorousFlowInternalEnergyIdeal
    at_nodes = true
    specific_heat_capacity = 1.3
    phase = 0
  [../]
  [./internal_energy_fluids]
    type = PorousFlowJoiner
    include_old = false
    at_nodes = true
    material_property = PorousFlow_fluid_phase_internal_energy_nodal
  [../]
[]

[Postprocessors]
  [./total_heat]
    type = PorousFlowHeatEnergy
    kernel_variable_number = 2
  [../]
  [./rock_heat]
    type = PorousFlowHeatEnergy
  [../]
  [./fluid_heat]
    type = PorousFlowHeatEnergy
    include_porous_skeleton = false
    phase = 0
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1 1 10000'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = except01
  csv = true
[]
