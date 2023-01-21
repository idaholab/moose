# PorousFlowPeacemanBorehole exception test
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  zmin = -1
  zmax = 1
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
    type = TimeDerivative
    variable = pp
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [borehole_total_outflow_mass]
    type = PorousFlowSumQuantity
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
[]

[DiracKernels]
  [bh]
    type = PorousFlowPeacemanBorehole
    bottom_p_or_t = 0
    fluid_phase = 0
    point_file = coincident_points.bh
    SumQuantityUO = borehole_total_outflow_mass
    variable = pp
    unit_weight = '0 0 0'
    character = 1
  []
[]

[Postprocessors]
  [bh_report]
    type = PorousFlowPlotQuantity
    uo = borehole_total_outflow_mass
  []

  [fluid_mass0]
    type = PorousFlowFluidMass
    execute_on = timestep_begin
  []

  [fluid_mass1]
    type = PorousFlowFluidMass
    execute_on = timestep_end
  []

  [zmass_error]
    type = FunctionValuePostprocessor
    function = mass_bal_fcn
    execute_on = timestep_end
  []

  [p0]
    type = PointValue
    variable = pp
    point = '0 0 0'
    execute_on = timestep_end
  []
[]

[Functions]
  [mass_bal_fcn]
    type = ParsedFunction
    expression = abs((a-c+d)/2/(a+c))
    symbol_names = 'a c d'
    symbol_values = 'fluid_mass1 fluid_mass0 bh_report'
  []
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

[Executioner]
  type = Transient
  end_time = 0.5
  dt = 1E-2
  solve_type = NEWTON
[]
