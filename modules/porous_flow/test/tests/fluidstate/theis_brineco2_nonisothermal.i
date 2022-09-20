# Two phase nonisothermal Theis problem: Flow from single source.
# Constant rate injection 2 kg/s of cold CO2 into warm reservoir
# 1D cylindrical mesh
# Initially, system has only a liquid phase, until enough gas is injected
# to form a gas phase, in which case the system becomes two phase.

[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 40
    xmin = 0.1
    xmax = 200
    bias_x = 1.05
  []
[]

[Problem]
  type = FEProblem
  coord_type = RZ
  rz_coord_axis = Y
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[AuxVariables]
  [saturation_gas]
    order = CONSTANT
    family = MONOMIAL
  []
  [x1]
    order = CONSTANT
    family = MONOMIAL
  []
  [y0]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [saturation_gas]
    type = PorousFlowPropertyAux
    variable = saturation_gas
    property = saturation
    phase = 1
    execute_on = timestep_end
  []
  [x1]
    type = PorousFlowPropertyAux
    variable = x1
    property = mass_fraction
    phase = 0
    fluid_component = 1
    execute_on = timestep_end
  []
  [y0]
    type = PorousFlowPropertyAux
    variable = y0
    property = mass_fraction
    phase = 1
    fluid_component = 0
    execute_on = timestep_end
  []
[]

[Variables]
  [pgas]
    initial_condition = 20e6
  []
  [zi]
    initial_condition = 0
  []
  [xnacl]
    initial_condition = 0.1
  []
  [temperature]
    initial_condition = 70
    scaling = 1e-4
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
  [mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = zi
  []
  [flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = zi
  []
  [mass2]
    type = PorousFlowMassTimeDerivative
    fluid_component = 2
    variable = xnacl
  []
  [flux2]
    type = PorousFlowAdvectiveFlux
    fluid_component = 2
    variable = xnacl
  []
  [energy]
    type = PorousFlowEnergyTimeDerivative
    variable = temperature
  []
  [heatadv]
    type = PorousFlowHeatAdvection
    variable = temperature
  []
  [conduction]
    type = PorousFlowHeatConduction
    variable = temperature
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pgas zi xnacl temperature'
    number_fluid_phases = 2
    number_fluid_components = 3
  []
  [pc]
    type = PorousFlowCapillaryPressureConst
    pc = 0
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
    z = zi
    temperature = temperature
    temperature_unit = Celsius
    xnacl = xnacl
    capillary_pressure = pc
    fluid_state = fs
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.2
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-12 0 0 0 1e-12 0 0 0 1e-12'
  []
  [relperm_water]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
    s_res = 0.1
    sum_s_res = 0.1
  []
  [relperm_gas]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 1
  []
  [rockheat]
    type = PorousFlowMatrixInternalEnergy
    specific_heat_capacity = 1000
    density = 2500
  []
  [rock_thermal_conductivity]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '50 0 0  0 50 0  0 0 50'
  []
[]

[BCs]
  [cold_gas]
    type = DirichletBC
    boundary = left
    variable = temperature
    value = 20
  []
  [gas_injecton]
    type = PorousFlowSink
    boundary = left
    variable = zi
    flux_function = -0.159155
  []
  [rightwater]
    type = DirichletBC
    boundary = right
    value = 20e6
    variable = pgas
  []
  [righttemp]
    type = DirichletBC
    boundary = right
    value = 70
    variable = temperature
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = 'gmres      asm      lu           NONZERO                   2'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  end_time = 1e4
  nl_abs_tol = 1e-7
  nl_rel_tol = 1e-5
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1
    growth_factor = 1.5
  []
[]

[Postprocessors]
  [pgas]
    type = PointValue
    point =  '2 0 0'
    variable = pgas
  []
  [sgas]
    type = PointValue
    point =  '2 0 0'
    variable = saturation_gas
  []
  [zi]
    type = PointValue
    point = '2 0 0'
    variable = zi
  []
  [temperature]
    type = PointValue
    point = '2 0 0'
    variable = temperature
  []
  [massgas]
    type = PorousFlowFluidMass
    fluid_component = 1
  []
  [x1]
    type = PointValue
    point =  '2 0 0'
    variable = x1
  []
  [y0]
    type = PointValue
    point =  '2 0 0'
    variable = y0
  []
[]

[Outputs]
  print_linear_residuals = false
  perf_graph = true
  csv = true
[]
