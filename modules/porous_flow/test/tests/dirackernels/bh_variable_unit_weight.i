# PorousFlowPeacemanBorehole with the wellbore pressure built from a temperature-dependent
# fluid density instead of a constant unit_weight.
#
# Same small-scale well/mesh/well_constant setup as bh_unit_weight.i (see the comment there for
# why both the mesh is kept small and why the fluxes below can be computed by hand from
# pp ~ 1E7 without needing to solve for pp).  Here an imposed linear temperature gradient
# T(z) = 300 - 5*z (Kelvin) drives a SimpleFluidProperties density
# rho(z) = density0 * exp(unit_weight_reference_pressure/bulk_modulus - thermal_expansion*T(z))
# at each of the 4 well points (z-centres 1.5, 0.5, -0.5, -1.5), and the wellbore pressure is
# obtained by trapezoidal-rule integration of rho*gravity from the bottom point (z=-1.5, where
# bottom_p_or_t=9.9E6 is defined) up to each point:
#   bh_pressure(z_bottom) = 9.9E6
#   bh_pressure(z_i) = bh_pressure(z_{i+1}) + 0.5*(rho_i + rho_{i+1})*gravity_z*(z_i - z_{i+1})
# giving T = (292.5, 297.5, 302.5, 307.5) K, rho = (1119.65, 1114.06, 1108.51, 1102.98) kg/m^3,
# bh_pressure = (9.867295E6, 9.878251E6, 9.889153E6, 9.9E6), and
# flux = n_i * well_constant * (pp - bh_pressure_i) = (1.327E-7, 2.435E-7, 2.217E-7, 1.0E-7) kg/s,
# total = 6.979E-7 kg/s (n_i = 1,2,2,1 as in bh_unit_weight.i).
# With thermal_expansion=0 (see the derived 'bh_variable_unit_weight_isothermal' test in the
# 'tests' spec file), rho is constant and this reduces exactly to the constant-unit_weight
# formula in bh_unit_weight.i, with unit_weight = (0,0,rho*gravity_z).
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 4
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
  zmin = -2
  zmax = 2
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [pp]
    initial_condition = 1E7
  []
[]

[AuxVariables]
  [T]
  []
[]

[AuxKernels]
  [T_aux]
    type = FunctionAux
    variable = T
    function = temp_fcn
    execute_on = 'initial timestep_begin'
  []
[]

[Functions]
  [temp_fcn]
    type = ParsedFunction
    expression = '300 - 5 * z'
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
[]

[UserObjects]
  [borehole_total_outflow_mass]
    type = PorousFlowSumQuantity
  []
  [borehole_point_fluxes]
    type = PorousFlowPointFluxQuantity
  []
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1e-7
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2e9
    viscosity = 1e-3
    density0 = 1500
    thermal_expansion = 1e-3
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = T
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
    permeability = '1E-12 0 0 0 1E-12 0 0 0 1E-12'
  []
  [relperm]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
  []
[]

[DiracKernels]
  [bh]
    type = PorousFlowPeacemanBorehole
    variable = pp
    SumQuantityUO = borehole_total_outflow_mass
    PointFluxUO = borehole_point_fluxes
    point_file = bh_vertical.bh
    function_of = pressure
    fluid_phase = 0
    bottom_p_or_t = 9.9E6
    unit_weight_fp = simple_fluid
    unit_weight_temperature = T
    unit_weight_gravity = '0 0 -9.81'
    unit_weight_reference_pressure = 1E5
    well_constant = 1E-12
    use_mobility = false
    character = 1
  []
[]

[Postprocessors]
  [bh_report]
    type = PorousFlowPlotQuantity
    uo = borehole_total_outflow_mass
  []
[]

[VectorPostprocessors]
  [point_fluxes]
    type = PorousFlowPlotPointFluxQuantity
    uo = borehole_point_fluxes
  []
[]

[Executioner]
  type = Transient
  end_time = 1
  dt = 1
  solve_type = NEWTON
[]

[Preconditioning]
  [usual]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason'
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -ksp_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-10 1E-10 10000 30'
  []
[]

[Outputs]
  file_base = bh_variable_unit_weight
  exodus = false
  csv = true
  execute_on = timestep_end
[]
