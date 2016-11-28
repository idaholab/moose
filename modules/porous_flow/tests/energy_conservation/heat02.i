# checking that the heat-energy postprocessor correctly calculates the energy
# 1phase, constant porosity
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
  [./temp]
  [../]
  [./pp]
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
    function = 'x'
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
    temperature = temp
  [../]
  [./nnn]
    type = PorousFlowNodeNumber
    on_initial_only = true
  [../]
  [./porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  [../]
  [./rock_heat]
    type = PorousFlowMatrixInternalEnergy
    specific_heat_capacity = 2.2
    density = 0.5
  [../]
  [./ppss]
    type = PorousFlow1PhaseP_VG
    porepressure = pp
    al = 1
    m = 0.5
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    density_P0 = 1
    bulk_modulus = 1
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    include_old = true
    material_property = PorousFlow_fluid_phase_density
  [../]
  [./water_heat]
    type = PorousFlowInternalEnergyIdeal
    specific_heat_capacity = 1.3
    phase = 0
  [../]
  [./internal_energy_fluids]
    type = PorousFlowJoiner
    include_old = true
    material_property = PorousFlow_fluid_phase_internal_energy_nodal
  [../]
[]

[Postprocessors]
  [./total_heat]
    type = PorousFlowHeatEnergy
    phase = 0
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
  file_base = heat02
  csv = true
[]
