# Gradients of the fluid state properties when the salt mass fraction and the temperature vary in
# space. Both are held fixed at their spatially varying initial values by TimeDerivative kernels.
# When they are listed in porous_flow_vars they are PorousFlow variables; when they are omitted
# they are treated in the same way as AuxVariables. The gradients of saturation, porepressure and
# mass fraction, and hence the solution, must be the same in both cases

[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
    xmax = 10
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [pgas]
    initial_condition = 10e6
  []
  [z]
    initial_condition = 0.3
  []
  [xnacl]
    [InitialCondition]
      type = FunctionIC
      function = '0.05 + 0.01 * x'
    []
  []
  [temperature]
    [InitialCondition]
      type = FunctionIC
      function = '30 + 2 * x'
    []
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pgas
  []
  [flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pgas
  []
  [diff0]
    type = PorousFlowDispersiveFlux
    fluid_component = 0
    variable = pgas
    disp_long = '0 0'
    disp_trans = '0 0'
  []
  [mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = z
  []
  [flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = z
  []
  [diff1]
    type = PorousFlowDispersiveFlux
    fluid_component = 1
    variable = z
    disp_long = '0 0'
    disp_trans = '0 0'
  []
  [xnacl]
    type = TimeDerivative
    variable = xnacl
  []
  [temperature]
    type = TimeDerivative
    variable = temperature
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pgas z xnacl temperature'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    alpha = 1e-5
    m = 0.5
  []
  [fs]
    type = PorousFlowBrineCO2
    brine_fp = brine
    co2_fp = co2
    capillary_pressure = pc
  []
[]

[FluidProperties]
  [co2]
    type = CO2FluidProperties
  []
  [brine]
    type = BrineFluidProperties
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = temperature
  []
  [brineco2]
    type = PorousFlowFluidState
    gas_porepressure = pgas
    z = z
    temperature = temperature
    temperature_unit = Celsius
    xnacl = xnacl
    capillary_pressure = pc
    fluid_state = fs
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-12 0 0 0 1e-12 0 0 0 1e-12'
  []
  [relperm_water]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
  []
  [relperm_gas]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 1
  []
  [diffusivity]
    type = PorousFlowDiffusivityConst
    diffusion_coeff = '1e-6 1e-6 1e-6 1e-6'
    tortuosity = '1 1'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 2500
  end_time = 1e4
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-12
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [pgas_2]
    type = PointValue
    variable = pgas
    point = '2.5 0 0'
  []
  [pgas_8]
    type = PointValue
    variable = pgas
    point = '7.5 0 0'
  []
  [z_2]
    type = PointValue
    variable = z
    point = '2.5 0 0'
  []
  [z_8]
    type = PointValue
    variable = z
    point = '7.5 0 0'
  []
[]

[Outputs]
  csv = true
[]
