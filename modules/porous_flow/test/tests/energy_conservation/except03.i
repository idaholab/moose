# Checking that the heat energy postprocessor correctly throws a paramError when an incorrect
# strain base_name is given

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
  [pp]
  []
  [temp]
  []
[]

[ICs]
  [tinit]
    type = FunctionIC
    function = '100*x'
    variable = temp
  []
  [pinit]
    type = FunctionIC
    function = x
    variable = pp
  []
[]

[Kernels]
  [dummyt]
    type = TimeDerivative
    variable = temp
  []
  [dummyp]
    type = TimeDerivative
    variable = pp
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'temp pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 1
    density0 = 1
    viscosity = 0.001
    thermal_expansion = 0
    cv = 1.3
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = temp
  []
  [porosity]
    type = PorousFlowPorosity
    porosity_zero = 0.1
  []
  [rock_heat]
    type = PorousFlowMatrixInternalEnergy
    specific_heat_capacity = 2.2
    density = 0.5
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
[]

[Postprocessors]
  [heat]
    type = PorousFlowHeatEnergy
    base_name = incorrect_base_name
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
  dt = 1
  end_time = 1
[]

[Outputs]
  execute_on = 'timestep_end'
[]
