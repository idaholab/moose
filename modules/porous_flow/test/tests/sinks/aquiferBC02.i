# Test 2 - Drainage to a lower aquifer
#
# A 1D horizontal saturated column is initially over-pressured at P = 2e6 Pa.
# PorousFlowAquiferBC at the right boundary (x=1) connects to an aquifer at
# pressure P_ref = 1e6 Pa (specified via aquifer_pressure_at_datum with gravity
# acting in z so the z=0 boundary has no hydrostatic correction).
#
# The Darcy flux kernel equilibrates pressure within the domain, so the
# system evolves as a lumped capacitor draining toward P_ref.  After many
# time constants the pressure converges to P_ref = 1e6 Pa throughout.
#
# Analytical time constant: tau = phi * rho * L_domain / (C * K_bulk)
# C = rho * k_nn / (mu * L_aq) = 1000*1e-10/(1e-3*20000) = 5e-9 kg/(m^2 Pa s)
# With phi=0.1, rho=1000, L_domain=1, C=5e-9, K_bulk=2e9: tau = 10 s.
# Running to t=1000 s >> tau gives P -> P_ref to machine precision.

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
  [aquifer_right]
    type = PorousFlowAquiferBC
    variable = pp
    boundary = 'right'
    fluid_phase = 0
    gravity = '0 0 -9.81'
    aquifer_pressure_at_datum = 1e6
    datum_elevation = 0
    aquifer_distance = 20000
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
  end_time = 1000
  nl_abs_tol = 1e-10
[]

[Outputs]
  file_base = aquiferBC02
  [csv]
    type = CSV
    execute_on = 'final'
  []
[]
