# PorousFlowPeacemanBorehole with a per-point well_constants override (constant unit_weight).
#
# Same small-scale well/mesh setup as bh_unit_weight.i (see the comment there for why the mesh
# is kept small, and for the bh_pressure derivation reused verbatim below) - the only change is
# replacing the single well-wide well_constant = 1E-12 with a well_constants vector giving each
# of the 4 points (z-centres 1.5, 0.5, -0.5, -1.5) its own value, 1E-12/2E-12/3E-12/4E-12
# respectively, exercising the new per-point W_i = well_constants[i] override
# (PorousFlowPeacemanBorehole::wellConstant()) instead of a single W for the whole well.
#
#   bh_pressure_i = bottom_p_or_t + unit_weight_z * (z_i - z_bottom)
#                 = 9.9E6 + (-1E4) * (z_i - (-1.5))
#   bh_pressure = (9.87E6, 9.88E6, 9.89E6, 9.9E6) for points (0,1,2,3) at z = (1.5, 0.5, -0.5, -1.5)
#
# The flux at each point reduces to n_i * well_constants[i] * (pp - bh_pressure_i), where n_i is
# the number of active half-segments at point i (1, 2, 2, 1 for the top, two interior, and
# bottom points respectively, as in bh_unit_weight.i) - so, with pp ~ 1E7:
#
#   flux_0 = 1 * 1E-12 * (1E7 - 9.87E6) = 1.3E-7 kg/s
#   flux_1 = 2 * 2E-12 * (1E7 - 9.88E6) = 4.8E-7 kg/s
#   flux_2 = 2 * 3E-12 * (1E7 - 9.89E6) = 6.6E-7 kg/s
#   flux_3 = 1 * 4E-12 * (1E7 - 9.9E6)  = 4.0E-7 kg/s
#   total = 16.7E-7 kg/s
#
# Compare to bh_unit_weight.i's uniform well_constant = 1E-12 case, flux = (1.3E-7, 2.4E-7,
# 2.2E-7, 1.0E-7) kg/s: point 0 is identical (well_constants[0] = 1E-12, same as there), while
# points 1-3 scale up in proportion to well_constants[i]/1E-12 = (2, 3, 4), demonstrating that
# each point's own entry - and only that entry - controls its own flux.
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
    density0 = 1000
    thermal_expansion = 0
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
    unit_weight = '0 0 -1E4'
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
  file_base = bh_well_constants
  exodus = false
  csv = true
  execute_on = timestep_end
[]
