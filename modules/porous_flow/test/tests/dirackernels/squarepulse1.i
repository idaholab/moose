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
  [pp]
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = pp
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1e-7
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2e9
    density0 = 1000
    thermal_expansion = 0
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow1PhaseP
    porepressure = pp
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.2
  []
[]

[Postprocessors]
  [total_mass]
    type = PorousFlowFluidMass
    execute_on = 'initial timestep_end'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  nl_abs_tol = 1e-14
  dt = 200
  end_time = 2000
[]

[Outputs]
  perf_graph = true
  file_base = squarepulse1
  csv = true
  execute_on = 'initial timestep_end'
  [con]
    output_linear = true
    type = Console
  []
[]

[ICs]
  [PressureIC]
    variable = pp
    type = ConstantIC
    value = 20e6
  []
[]

[DiracKernels]
  [sink1]
    type = PorousFlowSquarePulsePointSource
    start_time = 100
    end_time = 300
    point = '0.5 0.5 0'
    mass_flux = -0.1
    variable = pp
  []
  [sink]
    type = PorousFlowSquarePulsePointSource
    start_time = 600
    end_time = 1400
    point = '0.5 0.5 0'
    mass_flux = -0.1
    variable = pp
  []
  [source]
    point = '0.5 0.5 0'
    start_time = 1500
    mass_flux = 0.2
    end_time = 2000
    variable = pp
    type = PorousFlowSquarePulsePointSource
  []
[]
