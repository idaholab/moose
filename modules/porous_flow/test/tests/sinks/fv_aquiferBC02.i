# FV Test 2 - Drainage with computed conductance and the aquifer_permeability
# override (FV analogue of the aquiferBC06 pair)
#
# A 1D horizontal saturated column is initially over-pressured at P = 2e6 Pa.
# FVPorousFlowAquiferBC at the right boundary connects to an aquifer at
# P_ref = 1e6 Pa via the pressure-at-datum formulation.  The run is stopped at
# t = 20 s, well before full drainage, where the pressure still depends
# strongly on the conductance (p_right ~= 1.1975e6 Pa, vs P_ref = 1e6 Pa).
#
# The test spec runs this input twice:
#  (a) as-is: no aquifer_permeability, so the conductance uses the boundary
#      cell (mesh) permeability 1e-10 m^2;
#  (b) via cli_args: mesh permeability raised 100x to 1e-8 m^2, with
#      aquifer_permeability = 1e-10 m^2 supplied explicitly.
# Both runs must give the same pressures, demonstrating that
# aquifer_permeability overrides the boundary value and defaults to it when
# not supplied.

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
    initial_condition = 2e6
  []
[]

[FVKernels]
  [mass0]
    type = FVPorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
  [flux]
    type = FVPorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pp
    gravity = '0 0 0'
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
  [aquifer_right]
    type = FVPorousFlowAquiferBC
    variable = pp
    boundary = 'right'
    phase = 0
    fluid_component = 0
    gravity = '0 0 -9.81'
    aquifer_pressure_at_datum = 1e6
    datum_elevation = 0
    aquifer_distance = 20000
  []
[]

[Postprocessors]
  [p_left]
    type = PointValue
    point = '0.05 0 0'
    variable = pp
    execute_on = 'timestep_end'
  []
  [p_right]
    type = PointValue
    point = '0.95 0 0'
    variable = pp
    execute_on = 'timestep_end'
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
  dt = 5
  end_time = 20
  nl_abs_tol = 1e-10
[]

[Outputs]
  file_base = fv_aquiferBC02
  [csv]
    type = CSV
    execute_on = 'final'
  []
[]
