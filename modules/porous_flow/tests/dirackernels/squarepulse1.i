# Test PorousFlowSquarePulsePointSource DiracKernel

[Mesh]
  type = GeneratedMesh
  dim = 2
  bias_x = 1.1
  bias_y = 1.1
  ymax = 1
  xmax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [./pp]
  [../]
[]

[Kernels]
  [./mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = pp
    number_fluid_phases = 1
    number_fluid_components = 1
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
    at_nodes = true
  [../]
  [./ppss]
    type = PorousFlow1PhaseP_VG
    at_nodes = true
    porepressure = pp
    al = 1E-7
    m = 0.5
  [../]
  [./massfrac]
    type = PorousFlowMassFraction
    at_nodes = true
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 1000
    bulk_modulus = 2E9
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    include_old = true
    at_nodes = true
    material_property = PorousFlow_fluid_phase_density_nodal
  [../]
  [./porosity]
    type = PorousFlowPorosityConst
    at_nodes = true
    porosity = 0.2
  [../]
[]

[Postprocessors]
  [./total_mass]
    type = PorousFlowFluidMass
    execute_on = 'initial timestep_end'
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 200
  end_time = 2000
[]

[Outputs]
  print_perf_log = true
  file_base = squarepulse1
  csv = true
  execute_on = 'initial timestep_end'
  [./con]
    output_linear = true
    type = Console
  [../]
[]

[ICs]
  [./PressureIC]
    variable = pp
    type = ConstantIC
    value = 20e6
  [../]
[]

[DiracKernels]
  [./sink1]
    type = PorousFlowSquarePulsePointSource
    start_time = 100
    end_time = 300
    point = '0.5 0.5 0'
    mass_flux = -0.1
    variable = pp
  [../]
  [./sink]
    type = PorousFlowSquarePulsePointSource
    start_time = 600
    end_time = 1400
    point = '0.5 0.5 0'
    mass_flux = -0.1
    variable = pp
  [../]
  [./source]
    point = '0.5 0.5 0'
    start_time = 1500
    mass_flux = 0.2
    end_time = 2000
    variable = pp
    type = PorousFlowSquarePulsePointSource
  [../]
[]
