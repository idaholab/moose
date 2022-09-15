# Newton cooling from a bar.  1-phase ideal fluid and heat, steady
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 1
  xmin = 0
  xmax = 100
  ymin = 0
  ymax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pressure temp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.8
    alpha = 1e-5
  []
[]

[Variables]
  [pressure]
  []
  [temp]
  []
[]

[ICs]
  # have to start these reasonably close to their steady-state values
  [pressure]
    type = FunctionIC
    variable = pressure
    function = '200-0.5*x'
  []
  [temperature]
    type = FunctionIC
    variable = temp
    function = 180+0.1*x
  []
[]

[Kernels]
  [flux]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    gravity = '0 0 0'
    variable = pressure
  []
  [heat_advection]
    type = PorousFlowHeatAdvection
    gravity = '0 0 0'
    variable = temp
  []
[]

[FluidProperties]
  [idealgas]
    type = IdealGasFluidProperties
    molar_mass = 1.4
    gamma = 1.2
    mu = 1.2
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = temp
  []
  [ppss]
    type = PorousFlow1PhaseP
    porepressure = pressure
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
  []
  [dens0]
    type = PorousFlowSingleComponentFluid
    fp = idealgas
    phase = 0
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1.1 0 0 0 1.1 0 0 0 1.1'
  []
  [relperm]
    type = PorousFlowRelativePermeabilityCorey # irrelevant in this fully-saturated situation
    n = 2
    phase = 0
  []
[]

[BCs]
  [leftp]
    type = DirichletBC
    variable = pressure
    boundary = left
    value = 200
  []
  [leftt]
    type = DirichletBC
    variable = temp
    boundary = left
    value = 180
  []
  [newtonp]
    type = PorousFlowPiecewiseLinearSink
    variable = pressure
    boundary = right
    pt_vals = '-200 0 200'
    multipliers = '-200 0 200'
    use_mobility = true
    use_relperm = true
    fluid_phase = 0
    flux_function = 0.005 # 1/2/L
  []
  [newtont]
    type = PorousFlowPiecewiseLinearSink
    variable = temp
    boundary = right
    pt_vals = '-200 0 200'
    multipliers = '-200 0 200'
    use_mobility = true
    use_relperm = true
    use_enthalpy = true
    fluid_phase = 0
    flux_function = 0.005 # 1/2/L
  []
[]

[VectorPostprocessors]
  [porepressure]
    type = LineValueSampler
    variable = pressure
    start_point = '0 0.5 0'
    end_point = '100 0.5 0'
    sort_by = x
    num_points = 11
    execute_on = timestep_end
  []
  [temperature]
    type = LineValueSampler
    variable = temp
    start_point = '0 0.5 0'
    end_point = '100 0.5 0'
    sort_by = x
    num_points = 11
    execute_on = timestep_end
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = Newton
  nl_rel_tol = 1E-10
  nl_abs_tol = 1E-15
[]

[Outputs]
  file_base = nc08
  execute_on = timestep_end
  [along_line]
    type = CSV
    execute_vector_postprocessors_on = timestep_end
  []
[]
