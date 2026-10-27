# Test 4 - Steady-state convergence and near-equivalence with PorousFlowPiecewiseLinearSink
#
# On a purely horizontal 1D domain at z=0 with gravity='0 0 -9.81':
#   P_aq(z=0) = rho_nodal * g * h_aq
#
# Because rho_nodal depends on P, the steady-state satisfies:
#   P* = rho(P*) * 9.81 * 10
#
# For SimpleFluidProperties with bulk_modulus=2e9, density0=1000:
#   P* ~ 98104.8 Pa  (slightly above the incompressible value of 98100 Pa)
#
# The companion test (aquiferBC04_pls.i) uses PorousFlowPiecewiseLinearSink with
# a fixed PT_shift=98100, converging to exactly 98100 Pa.  The ~4.8 Pa difference
# is well within the rel_err=1e-4 tolerance (9.8 Pa), confirming near-equivalence.

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
    initial_condition = 2e6
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
  [darcy]
    type = PorousFlowAdvectiveFlux
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
    type = PorousFlowTemperature
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
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-10 0 0  0 1e-10 0  0 0 1e-10'
  []
  [relperm]
    type = PorousFlowRelativePermeabilityCorey
    n = 0
    phase = 0
  []
[]

[BCs]
  [right_bc]
    type = PorousFlowAquiferBC
    variable = pp
    boundary = 'right'
    fluid_phase = 0
    gravity = '0 0 -9.81'
    aquifer_head = 10.0
    aquifer_conductance = 5e-9
  []
[]

[Postprocessors]
  [p_left]
    type = PointValue
    point = '0 0 0'
    variable = pp
    execute_on = 'timestep_end'
  []
  [p_right]
    type = PointValue
    point = '1 0 0'
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
  dt = 50
  end_time = 500
  nl_abs_tol = 1e-10
[]

[Outputs]
  file_base = aquiferBC04
  [csv]
    type = CSV
    execute_on = 'final'
  []
[]
