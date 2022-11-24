# Test PorousFlowPointSourceFromPostprocessor DiracKernel

[Mesh]
  type = GeneratedMesh
  dim = 2
  bias_x = 1.1
  bias_y = 1.1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Functions]
  [mass_flux_fn]
    type = PiecewiseConstant
    direction = left
    xy_data = '
      0    0
      100  -0.1
      300  0
      600  -0.1
      1400 0
      1500 0.2
      2000 0.2'
  []
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
    type = PorousFlow1PhaseFullySaturated
    porepressure = pp
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

  [mass_flux_in]
    type = FunctionValuePostprocessor
    function = mass_flux_fn
    execute_on = 'initial timestep_begin'
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
  dt = 100
  end_time = 2000
[]

[Outputs]
  perf_graph = true
  csv = true
  execute_on = 'initial timestep_end'
  file_base = frompps
[]

[ICs]
  [PressureIC]
    variable = pp
    type = ConstantIC
    value = 20e6
  []
[]

[DiracKernels]
  [source]
    type = PorousFlowPointSourceFromPostprocessor
    variable = pp
    mass_flux = mass_flux_in
    point = '0.5 0.5 0'
  []
[]
