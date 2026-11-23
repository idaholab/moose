# PorousFlowPeacemanBorehole combining a per-point well_constants override WITH the
# temperature-dependent wellbore-pressure profile (unit_weight_fp) - the combination this
# feature exists for: an independent well constant at each point, while still building the
# wellbore pressure from the true cumulative integral over the whole well (see sinks.md,
# "A well constant that varies along the well").
#
# Identical setup to bh_variable_unit_weight.i (same mesh, same imposed temperature gradient
# T(z) = 300 - 5*z, same fluid properties) EXCEPT well_constant = 1E-12 there becomes
# well_constants = '1E-12 2E-12 3E-12 4E-12' here - i.e. only the well-index side of the
# calculation changes; the wellbore-pressure side is untouched, and should give IDENTICAL
# bh_pressure values to bh_variable_unit_weight.i, since well_constants does not participate in
# that calculation at all (see PorousFlowPeacemanBorehole::computeWellborePressures(), which
# never reads well_constant or well_constants).
#
# Reusing bh_variable_unit_weight.i's own derivation verbatim for bh_pressure:
#   T = (292.5, 297.5, 302.5, 307.5) K, rho = (1119.65, 1114.06, 1108.51, 1102.98) kg/m^3
#   bh_pressure(z_bottom=-1.5) = 9.9E6
#   bh_pressure(z_i) = bh_pressure(z_{i+1}) + 0.5*(rho_i + rho_{i+1})*gravity_z*(z_i - z_{i+1})
#   bh_pressure = (9.867295E6, 9.878251E6, 9.889153E6, 9.9E6) for points (0,1,2,3)
#
# and, with pp ~ 1E7, n_i = (1,2,2,1) as in bh_unit_weight.i:
#   flux_i = n_i * well_constants[i] * (pp - bh_pressure_i)
#   flux_0 = 1 * 1E-12 * (1E7 - 9.867295E6) = 1.327E-7 kg/s   [identical to
#                                                               bh_variable_unit_weight.i's flux_0,
#                                                               since well_constants[0] = 1E-12
#                                                               there too]
#   flux_1 = 2 * 2E-12 * (1E7 - 9.878251E6) = 4.870E-7 kg/s
#   flux_2 = 2 * 3E-12 * (1E7 - 9.889153E6) = 6.651E-7 kg/s
#   flux_3 = 1 * 4E-12 * (1E7 - 9.9E6)       = 4.0E-7 kg/s
#   total = 1.6848E-6 kg/s
#
# Point 0's flux matching bh_variable_unit_weight.i's exactly, while points 1-3 scale up with
# well_constants[i], confirms the two mechanisms (per-point well index, cumulative wellbore
# pressure) are independent and compose correctly.
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
    well_constants = '1E-12 2E-12 3E-12 4E-12'
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
  nl_abs_tol = 1E-10
  nl_rel_tol = 1E-10
  nl_max_its = 10000
[]

[Preconditioning]
  [usual]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason'
    petsc_options_iname = '-ksp_type -pc_type -ksp_max_it'
    petsc_options_value = 'bcgs bjacobi 30'
  []
[]

[Outputs]
  file_base = bh_well_constants_variable_unit_weight
  exodus = false
  csv = true
  execute_on = timestep_end
[]
