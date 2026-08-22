# Test 5 - Hydrostatic residual on a vertical boundary
#
# Same idea as Test 1, but PorousFlowAquiferBC is applied to the right face
# (x = 1), which is a vertical boundary where elevation y ranges from 0 to 10 m.
# Each quadrature point on the face is at a different elevation, so this test
# directly exercises the per-QP elevation correction.
#
# The domain is initialised at hydrostatic equilibrium with water-table head
# h = 10 m (gravity in -y): P(y) = rho*g*(10-y) = 9810*(10-y).
# Because P_model(y) = P_aq(y) = rho*g*(10-y), the flux is identically zero at
# every quadrature point and the pressure field must remain unchanged after one
# timestep.
#
# PorousFlowPiecewiseLinearSink with a scalar PT_shift would fail this test:
# the scalar matches the aquifer pressure at only one elevation.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 10
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
  []
[]

[ICs]
  [pp]
    type = FunctionIC
    variable = pp
    function = '9810*(10-y)'
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    # Effectively incompressible (rho = 1000 kg/m^3 to machine precision for these pressures)
    # so that the linear IC P = rho0*g*(h-y) is the exact hydrostatic solution and
    # the nodal density used for p_ref also equals rho0 exactly.
    bulk_modulus = 1e50
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
    permeability = '1e-15 0 0  0 1e-15 0  0 0 1e-15'
  []
[]

[BCs]
  [aquifer_right]
    type = PorousFlowAquiferBC
    variable = pp
    boundary = 'right'
    fluid_phase = 0
    gravity = '0 -9.81 0'
    aquifer_head = 10.0
    aquifer_conductance = 1e-5
  []
[]

[Postprocessors]
  [p_bottom]
    type = PointValue
    point = '1 1 0'
    variable = pp
    execute_on = 'initial timestep_end'
  []
  [p_mid]
    type = PointValue
    point = '1 5 0'
    variable = pp
    execute_on = 'initial timestep_end'
  []
  [p_top]
    type = PointValue
    point = '1 9 0'
    variable = pp
    execute_on = 'initial timestep_end'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -pc_factor_shift_type'
    petsc_options_value = 'gmres lu NONZERO'
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
  file_base = aquiferBC05
  [csv]
    type = CSV
    execute_on = 'initial timestep_end'
  []
[]
