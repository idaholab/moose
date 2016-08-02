# checking that the mass postprocessor correctly calculates the mass
# of each component in each phase
# 2phase, 2component, constant porosity

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
  xmin = 0
  xmax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [./pp]
  [../]
  [./sat]
  [../]
[]

[AuxVariables]
  [./massfrac_ph0_sp0]
    initial_condition = 0.3
  [../]
  [./massfrac_ph1_sp0]
    initial_condition = 0.55
  [../]
[]

[ICs]
  [./pinit]
    type = ConstantIC
    value = 1
    variable = pp
  [../]
  [./satinit]
    type = FunctionIC
    function = 1-x
    variable = sat
  [../]
[]

[Kernels]
  [./mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  [../]
  [./mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = sat
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp sat'
    number_fluid_phases = 2
    number_fluid_components = 2
  [../]
[]

[Materials]
  [./nnn]
    type = PorousFlowNodeNumber
    on_initial_only = true
  [../]
  [./ppss]
    type = PorousFlow2PhasePS_VG
    phase0_porepressure = pp
    phase1_saturation = sat
    m = 0.5
    pc_max = 0
    sat_lr = 0
    sat_ls = 1
    p0 = 1
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'massfrac_ph0_sp0 massfrac_ph1_sp0'
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    density_P0 = 1
    bulk_modulus = 1
    phase = 0
  [../]
  [./dens1]
    type = PorousFlowDensityConstBulk
    density_P0 = 0.1
    bulk_modulus = 1
    phase = 1
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    include_old = true
    material_property = PorousFlow_fluid_phase_density
  [../]
  [./porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  [../]
[]

[Postprocessors]
  [./comp0_phase0_mass]
    type = PorousFlowFluidMass
    fluid_component = 0
    phase = 0
  [../]
  [./comp0_phase1_mass]
    type = PorousFlowFluidMass
    fluid_component = 0
    phase = 1
  [../]
  [./comp0_total_mass]
    type = PorousFlowFluidMass
    fluid_component = 0
  [../]
  [./comp1_phase0_mass]
    type = PorousFlowFluidMass
    fluid_component = 1
    phase = 0
  [../]
  [./comp1_phase1_mass]
    type = PorousFlowFluidMass
    fluid_component = 1
    phase = 1
  [../]
  [./comp1_total_mass]
    type = PorousFlowFluidMass
    fluid_component = 1
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
  file_base = fluidmass1
  csv = true
[]
