# Two phase Theis problem: Flow from single source.
# Constant rate injection 2 kg/s
# 1D cylindrical mesh
# Initially, system has only a liquid phase, until enough gas is injected
# to form a gas phase, in which case the system becomes two phase.
#
# This test takes a few minutes to run, so is marked heavy

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2000
  xmax = 2000
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
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pgas zi xnacl'
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
  [co2sw]
    type = CO2FluidProperties
  []
  [co2]
    type = TabulatedFluidProperties
    fp = co2sw
  []
  [water]
    type = Water97FluidProperties
  []
  [watertab]
    type = TabulatedFluidProperties
    fp = water
    temperature_min = 273.15
    temperature_max = 573.15
    fluid_property_file = water_fluid_properties.csv
    save_file = false
  []
  [brine]
    type = BrineFluidProperties
    water_fp = watertab
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = 20
  []
  [brineco2]
    type = PorousFlowFluidState
    gas_porepressure = pgas
    z = zi
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
[]

[BCs]
  [rightwater]
    type = DirichletBC
    boundary = right
    value = 20e6
    variable = pgas
  []
[]

[DiracKernels]
  [source]
    type = PorousFlowSquarePulsePointSource
    point = '0 0 0'
    mass_flux = 2
    variable = zi
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
  solve_type = NEWTON
  end_time = 1e5
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1
    growth_factor = 1.5
  []
[]

[VectorPostprocessors]
  [line]
    type = LineValueSampler
    warn_discontinuous_face_values = false
    sort_by = x
    start_point = '0 0 0'
    end_point = '2000 0 0'
    num_points = 10000
    variable = 'pgas zi xnacl x1 saturation_gas'
    execute_on = 'timestep_end'
  []
[]

[Postprocessors]
  [pgas]
    type = PointValue
    point = '4 0 0'
    variable = pgas
  []
  [sgas]
    type = PointValue
    point = '4 0 0'
    variable = saturation_gas
  []
  [zi]
    type = PointValue
    point = '4 0 0'
    variable = zi
  []
  [massgas]
    type = PorousFlowFluidMass
    fluid_component = 1
  []
  [x1]
    type = PointValue
    point = '4 0 0'
    variable = x1
  []
  [y0]
    type = PointValue
    point = '4 0 0'
    variable = y0
  []
  [xnacl]
    type = PointValue
    point = '4 0 0'
    variable = xnacl
  []
[]

[Outputs]
  print_linear_residuals = false
  perf_graph = true
  [csvout]
    type = CSV
    execute_on = timestep_end
    execute_vector_postprocessors_on = final
  []
[]
