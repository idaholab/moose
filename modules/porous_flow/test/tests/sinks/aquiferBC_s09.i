# Equivalent of s09.i using PorousFlowAquiferBC instead of PorousFlowPiecewiseLinearSink.
#
# The right-hand PLS in s09 keeps porepressure ~ 0 using:
#   flux_function = 1e4, pt_vals = '-100 100', multipliers = '-1 1'
# which encodes flux = 1e4 * (P/100) = 100 * P  [effective conductance = 100 kg/(m^2 Pa s)]
#
# PorousFlowAquiferBC with gravity = '0 0 0' gives P_aq = 0 regardless of aquifer_head,
# so flux = aquifer_conductance * (P - 0) = 100 * P -- identical to the PLS.
#
# Because the mapping is exact (same residual and Jacobian), this test produces
# bit-for-bit identical output to s09.i and is compared against the same gold data.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
  xmin = 0
  xmax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp frac'
    number_fluid_phases = 1
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1
  []
[]

[Variables]
  [pp]
  []
  [frac]
  []
[]

[ICs]
  [pp]
    type = FunctionIC
    variable = pp
    function = 1-x
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = frac
  []
  [mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = pp
  []
  [flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    gravity = '0 0 0'
    variable = frac
  []
  [flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    gravity = '0 0 0'
    variable = pp
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 1e10
    density0 = 1
    thermal_expansion = 0
    viscosity = 11
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
    mass_fraction_vars = frac
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
    permeability = '1.1 0 0 0 1.1 0 0 0 1.1'
  []
  [relperm]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
  []
[]

[BCs]
  [lhs_fixed_a]
    type = DirichletBC
    boundary = 'left'
    variable = frac
    value = 1
  []
  [lhs_fixed_b]
    type = DirichletBC
    boundary = 'left'
    variable = pp
    value = 1
  []
  # PorousFlowAquiferBC with gravity='0 0 0' gives P_aq=0, so flux = conductance * P.
  # conductance = 1e4 * (slope of PLS) = 1e4 * (1/100) = 100 kg/(m^2 Pa s).
  [flux0]
    type = PorousFlowAquiferBC
    boundary = 'right'
    variable = frac
    fluid_phase = 0
    mass_fraction_component = 0
    gravity = '0 0 0'
    aquifer_head = 0
    aquifer_conductance = 100
  []
  [flux1]
    type = PorousFlowAquiferBC
    boundary = 'right'
    variable = pp
    fluid_phase = 0
    mass_fraction_component = 1
    gravity = '0 0 0'
    aquifer_head = 0
    aquifer_conductance = 100
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -snes_max_it -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = 'gmres asm lu 10000 NONZERO 2'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1E-2
  end_time = 1
  nl_rel_tol = 1E-12
  nl_abs_tol = 1E-12
[]

[VectorPostprocessors]
  [mf]
    type = LineValueSampler
    start_point = '0 0 0'
    end_point = '1 0 0'
    num_points = 100
    sort_by = x
    variable = frac
  []
[]

[Outputs]
  file_base = aquiferBC_s09
  [console]
    type = Console
    execute_on = 'nonlinear linear'
  []
  [csv]
    type = CSV
    sync_times = '0.1 0.5 1'
    sync_only = true
  []
  time_step_interval = 10
[]
