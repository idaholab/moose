# Liquid and gas properties for code intercomparison problem 3
#
# From Pruess et al, Code intercomparison builds confidence in
# numerical simulation models for geologic disposal of CO2, Energy 29 (2004)
#
# This test simply calculates density and viscosity of each phase for
# various pressures and salinities, as well as mass fractions of CO2 in the
# liquid phase and H2O in the gas phase.
#
# Four versions of this are run:
# 1) No CO2, 0 salt mass fraction (pure water)
# 2) Enough CO2 to form gas phase, 0 salt mass fraction (pure water)
# 3) No CO2, 0.15 salt mass fraction
# 4) Enough CO2 to form gas phase, 0.15 salt mass fraction
#
# These results compare well with detailed results presented in Pruess et al,
# Intercomparison of numerical simulation codes for geologic disposal of CO2,
# LBNL-51813 (2002)

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  xmax = 4
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[AuxVariables]
  [density_liquid]
    order = CONSTANT
    family = MONOMIAL
  []
  [density_gas]
    order = CONSTANT
    family = MONOMIAL
  []
  [viscosity_liquid]
    order = CONSTANT
    family = MONOMIAL
  []
  [viscosity_gas]
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
  [xnacl]
    initial_condition = 0.0
  []
[]

[AuxKernels]
  [density_liquid]
    type = PorousFlowPropertyAux
    variable = density_liquid
    property = density
    phase = 0
    execute_on = timestep_end
  []
  [density_gas]
    type = PorousFlowPropertyAux
    variable = density_gas
    property = density
    phase = 1
    execute_on = timestep_end
  []
  [viscosity_liquid]
    type = PorousFlowPropertyAux
    variable = viscosity_liquid
    property = viscosity
    phase = 0
    execute_on = timestep_end
  []
  [viscosity_gas]
    type = PorousFlowPropertyAux
    variable = viscosity_gas
    property = viscosity
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
    order = CONSTANT
    family = MONOMIAL
  []
  [zi]
    initial_condition = 0.0
  []
[]

[Functions]
  [pic]
    type = ParsedFunction
    expression = 'if(x<1,12e6,if(x<2,16e6,if(x<3,20e6,24e6)))'
  []
[]

[ICs]
  [pic]
    type = FunctionIC
    function = pic
    variable = pgas
  []
[]

[Kernels]
  [diffusionp]
    type = NullKernel
    variable = pgas
  []
  [diffusionz]
    type = NullKernel
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
    temperature = 45
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
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = 'gmres      asm      lu           NONZERO                   2             '
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  perf_graph = true
  csv = true
  execute_on = timestep_end
  file_base = properties_water
[]

[VectorPostprocessors]
  [vpp]
    type = ElementValueSampler
    variable = 'pgas density_liquid density_gas viscosity_liquid viscosity_gas x1 y0'
    sort_by = x
  []
[]
