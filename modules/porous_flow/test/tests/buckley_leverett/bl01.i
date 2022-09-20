# Buckley-Leverett 1-phase.
# The front starts at (around) x=5, and at t=50 it should
# have moved to x=9.6.  The version below has a nonzero
# suction function, and at t=50, the front sits between
# (about) x=9.6 and x=9.9.  Changing the van-Genuchten
# al parameter to 1E-4 softens the front so it sits between
# (about) x=9.7 and x=10.4, and the simulation runs much faster.
# With al=1E-2 and nx=600, the front sits between x=9.6 and x=9.8,
# but takes about 100 times longer to run.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 150
  xmin = 0
  xmax = 15
[]

[GlobalParams]
  PorousFlowDictator = dictator
  compute_enthalpy = false
  compute_internal_energy = false
[]

[Variables]
  [pp]
    [InitialCondition]
      type = FunctionIC
      function = 'max((1000000-x/5*1000000)-20000,-20000)'
    []
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
  [flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pp
    gravity = '0 0 0'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = pp
    boundary = left
    value = 980000
  []
[]

[AuxVariables]
  [sat]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [sat]
    type = MaterialStdVectorAux
    variable = sat
    execute_on = timestep_end
    index = 0
    property = PorousFlow_saturation_qp
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.8
    alpha = 1e-3
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2e6
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
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-10 0 0  0 1E-10 0  0 0 1E-10'
  []
  [relperm]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.15
  []
[]

[Preconditioning]
  active = andy
  [andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'gmres bjacobi 1E-10 1E-10 20'
  []
[]

[Functions]
  [timestepper]
    type = PiecewiseLinear
    x = '0    0.01 0.1 1   1.5 2   20  30  40  50'
    y = '0.01 0.1  0.2 0.3 0.1 0.3 0.3 0.4 0.4 0.5'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 50
  [TimeStepper]
    type = FunctionDT
    function = timestepper
  []
[]

[VectorPostprocessors]
  [pp]
    type = LineValueSampler
    start_point = '0 0 0'
    end_point = '15 0 0'
    num_points = 150
    sort_by = x
    variable = pp
  []
  [sat]
    type = LineValueSampler
    warn_discontinuous_face_values = false
    start_point = '0 0 0'
    end_point = '15 0 0'
    num_points = 150
    sort_by = x
    variable = sat
  []
[]


[Outputs]
  file_base = bl01
  [csv]
    type = CSV
    sync_only = true
    sync_times = '0.01 50'
  []
  [exodus]
    type = Exodus
    execute_on = 'initial final'
  []
[]
