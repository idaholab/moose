# Test 6 - aquifer_permeability parameter and its default
#
# Same drainage setup as aquiferBC02, but stopped at t = 20 s, well before full
# drainage, where the pressure still depends strongly on the conductance
# (p_right ~= 1.1975e6 Pa, vs P_ref = 1e6 Pa).
#
# The test spec runs this input twice:
#  (a) as-is: no aquifer_permeability, so the conductance uses the boundary
#      (mesh) permeability 1e-10 m^2;
#  (b) via cli_args: mesh permeability raised 100x to 1e-8 m^2, with
#      aquifer_permeability = 1e-10 m^2 supplied explicitly.
# Both runs must give the same pressures, demonstrating that
# aquifer_permeability overrides the boundary value and defaults to it when
# not supplied.  If the override were ignored in run (b), the conductance
# would be 100x too large (tau = 0.1 s) and the domain would be fully drained
# to P_ref = 1e6 Pa by t = 20 s, a 13% discrepancy.

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
  dt = 5
  end_time = 20
  nl_abs_tol = 1e-10
[]

[Outputs]
  file_base = aquiferBC06
  [csv]
    type = CSV
    execute_on = 'final'
  []
[]
