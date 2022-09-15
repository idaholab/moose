# Tests that PorousFlow can successfully recover using a checkpoint file.
# This test contains stateful material properties, adaptivity and integrated
# boundary conditions with nodal-sized materials.
#
# This test file is run three times:
# 1) The full input file is run to completion
# 2) The input file is run for half the time and checkpointing is included
# 3) The input file is run in recovery using the checkpoint data
#
# The final output of test 3 is compared to the final output of test 1 to verify
# that recovery was successful.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
  xmax = 100
  bias_x = 1.05
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

[Adaptivity]
  marker = marker
  max_h_level = 4
  [Indicators]
    [front]
      type = GradientJumpIndicator
      variable = zi
    []
  []
  [Markers]
    [marker]
      type = ErrorFractionMarker
      indicator = front
      refine = 0.8
      coarsen = 0.2
    []
  []
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
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pgas zi'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureConst
    pc = 0
  []
  [fs]
    type = PorousFlowWaterNCG
    water_fp = water
    gas_fp = co2
    capillary_pressure = pc
  []
[]

[FluidProperties]
  [co2]
    type = CO2FluidProperties
  []
  [water]
    type = Water97FluidProperties
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = 20
  []
  [waterncg]
    type = PorousFlowFluidState
    gas_porepressure = pgas
    z = zi
    temperature_unit = Celsius
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
  [aquifer]
    type = PorousFlowPiecewiseLinearSink
    variable = pgas
    boundary = right
    pt_vals = '0 1e8'
    multipliers = '0 1e8'
    flux_function = 1e-6
    PT_shift = 20e6
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
  end_time = 2e2
  dt = 50
[]

[VectorPostprocessors]
  [line]
    type = NodalValueSampler
    sort_by = x
    variable = 'pgas zi'
  []
[]

[Outputs]
  print_linear_residuals = false
  perf_graph = true
  csv = true
[]
