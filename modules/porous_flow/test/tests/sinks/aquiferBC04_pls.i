# Test 4 (PiecewiseLinearSink variant) - see aquiferBC04.i for description.
#
# PorousFlowPiecewiseLinearSink with PT_shift = rho*g*h_aq = 98100 Pa,
# pt_vals/multipliers encoding f(x)=x, and flux_function = conductance.
# Must produce identical output to aquiferBC04.i.

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
    type = PorousFlowPiecewiseLinearSink
    variable = pp
    boundary = 'right'
    fluid_phase = 0
    pt_vals = '-1e9 1e9'
    multipliers = '-1e9 1e9'
    PT_shift = 98100
    flux_function = 5e-9
    use_mobility = false
    use_relperm = false
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
  file_base = aquiferBC04_pls
  [csv]
    type = CSV
    execute_on = 'final'
  []
[]
