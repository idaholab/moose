# Base input for FVPorousFlowAquiferBC exception tests.
#
# The FVBC block deliberately omits aquifer_head and aquifer_pressure_at_datum
# so that the default run triggers the "neither specified" error.
# Individual tests inject bad parameter combinations via cli_args.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0
  xmax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureConst
    pc = 0
  []
[]

[Variables]
  [pp]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 1e6
  []
[]

[FVKernels]
  [mass0]
    type = FVPorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2e9
    density0 = 1000
    thermal_expansion = 0
    viscosity = 1e-3
  []
[]

[Materials]
  [temperature]
    type = ADPorousFlowTemperature
  []
  [ppss]
    type = ADPorousFlow1PhaseP
    porepressure = pp
    capillary_pressure = pc
  []
  [massfrac]
    type = ADPorousFlowMassFraction
  []
  [simple_fluid]
    type = ADPorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [porosity]
    type = ADPorousFlowPorosityConst
    porosity = 0.1
  []
  [permeability]
    type = ADPorousFlowPermeabilityConst
    permeability = '1e-10 0 0  0 1e-10 0  0 0 1e-10'
  []
  [relperm]
    type = ADPorousFlowRelativePermeabilityConst
    kr = 1
    phase = 0
  []
[]

[FVBCs]
  [aquifer_exc]
    type = FVPorousFlowAquiferBC
    variable = pp
    boundary = 'right'
    phase = 0
    fluid_component = 0
    gravity = '0 0 -9.81'
    # No aquifer_head, aquifer_pressure_at_datum, aquifer_conductance, or
    # aquifer_distance: triggers "neither specified" error by default.
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
