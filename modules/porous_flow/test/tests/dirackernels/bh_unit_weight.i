# PorousFlowPeacemanBorehole with a nonzero constant unit_weight.
#
# A 4-point vertical well (bh_vertical.bh) passes through the centre of 4 stacked elements
# (z-centres 1.5, 0.5, -0.5, -1.5).  The element size is deliberately kept small (matching the
# scale of bh02.i) so that well_constant, set explicitly below to bypass the
# geometry/permeability-dependent well-constant calculation, gives a source term that is a small
# but numerically-resolvable perturbation on the reservoir pressure (on a much larger mesh, the
# same well_constant would extract a source so tiny relative to the reservoir's mass scale that
# it is swamped by floating-point noise, causing spurious Newton non-convergence).
# The flux at each point reduces to n_i * well_constant * (pp - bh_pressure_i), where n_i is the
# number of active half-segments at point i (1, 2, 2, 1 for the top, two interior, and bottom
# points respectively), so the expected fluxes below can be computed by hand:
#
#   bh_pressure_i = bottom_p_or_t + unit_weight_z * (z_i - z_bottom)
#                 = 9.9E6 + (-1E4) * (z_i - (-1.5))
#   bh_pressure = (9.87E6, 9.88E6, 9.89E6, 9.9E6) for points (0,1,2,3) at z = (1.5, 0.5, -0.5, -1.5)
#
#   flux_i = n_i * well_constant * (pp - bh_pressure_i), with pp ~ 1E7, well_constant = 1E-12
#   flux = (1.3E-7, 2.4E-7, 2.2E-7, 1.0E-7) kg/s, total = 6.9E-7 kg/s
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
  file_base = bh_unit_weight
  exodus = false
  csv = true
  execute_on = timestep_end
[]
