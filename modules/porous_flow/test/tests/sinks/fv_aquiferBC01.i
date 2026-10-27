# FV Test 1 - Hydrostatic residual (FV analogue of aquiferBC01)
#
# A vertical column (y-axis) is initialised at hydrostatic equilibrium with
# water-table head h = 10 m.  FVPorousFlowAquiferBC is applied on the right
# (vertical, cells at different elevations), bottom and top boundaries with
# aquifer_head = 10.0.
#
# Because P_cell(y) = rho*g*(10-y) at every cell centroid and the BC evaluates
# P_aq at the cell-centroid elevation, the flux is identically zero on every
# boundary face and the pressure field must remain unchanged after one
# timestep.  The horizontal (top/bottom) boundaries check that the BC uses the
# cell-centroid elevation rather than the face-centroid elevation, which sits
# half a cell above/below.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 5
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 10
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
  []
[]

[ICs]
  [pp]
    type = FunctionIC
    variable = pp
    function = '9810*(10-y)'
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
    gravity = '0 -9.81 0'
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    # Effectively incompressible (rho = 1000 kg/m^3 to machine precision for these
    # pressures) so that the linear IC P = rho0*g*(h-y) is the exact hydrostatic
    # solution and the cell density used for p_ref also equals rho0 exactly.
    bulk_modulus = 1e50
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
    permeability = '1e-15 0 0  0 1e-15 0  0 0 1e-15'
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
    gravity = '0 -9.81 0'
    aquifer_head = 10.0
    aquifer_conductance = 1e-5
  []
  [aquifer_bottom]
    type = FVPorousFlowAquiferBC
    variable = pp
    boundary = 'bottom'
    phase = 0
    fluid_component = 0
    gravity = '0 -9.81 0'
    aquifer_head = 10.0
    aquifer_conductance = 1e-5
  []
  [aquifer_top]
    type = FVPorousFlowAquiferBC
    variable = pp
    boundary = 'top'
    phase = 0
    fluid_component = 0
    gravity = '0 -9.81 0'
    aquifer_head = 10.0
    aquifer_conductance = 1e-5
  []
[]

[Postprocessors]
  [p_bottom]
    type = PointValue
    point = '0.5 1 0'
    variable = pp
    execute_on = 'initial timestep_end'
  []
  [p_mid]
    type = PointValue
    point = '0.5 5 0'
    variable = pp
    execute_on = 'initial timestep_end'
  []
  [p_top]
    type = PointValue
    point = '0.5 9 0'
    variable = pp
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
  dt = 1
  end_time = 1
  nl_abs_tol = 1e-12
[]

[Outputs]
  file_base = fv_aquiferBC01
  [csv]
    type = CSV
    execute_on = 'initial timestep_end'
  []
[]
