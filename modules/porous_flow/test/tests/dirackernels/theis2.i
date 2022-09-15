# Theis problem: Flow to single sink
# Constant rate injection between 200 and 1000 s.
# Cartesian mesh with logarithmic distribution in x and y.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  bias_x = 1.1
  bias_y = 1.1
  ymax = 100
  xmax = 100
[]

[GlobalParams]
  PorousFlowDictator = dictator
  compute_enthalpy = false
  compute_internal_energy = false
[]

[Variables]
  [pp]
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
  [flux]
    type = PorousFlowAdvectiveFlux
    variable = pp
    gravity = '0 0 0'
    fluid_component = 0
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
    m = 0.5
    alpha = 1e-7
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2e9
    density0 = 1000
    viscosity = 0.001
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
    porosity = 0.2
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-14 0 0 0 1E-14 0 0 0 1E-14'
  []
  [relperm]
    type = PorousFlowRelativePermeabilityCorey
    n = 0
    phase = 0
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
  dt = 200
  end_time = 1000
  nl_abs_tol = 1e-10
[]

[Outputs]
  perf_graph = true
  file_base = theis2
  [csv]
    type = CSV
    execute_on = final
  []
[]

[ICs]
  [PressureIC]
    variable = pp
    type = ConstantIC
    value = 20e6
  []
[]

[DiracKernels]
  [sink]
    type = PorousFlowSquarePulsePointSource
    start_time = 200
    end_time = 1000
    point = '0 0 0'
    mass_flux = -0.04
    variable = pp
  []
[]

[BCs]
  [right]
    type = DirichletBC
    variable = pp
    value = 20e6
    boundary = right
  []
  [top]
    type = DirichletBC
    variable = pp
    value = 20e6
    boundary = top
  []
[]

[VectorPostprocessors]
  [pressure]
    type = SideValueSampler
    variable = pp
    sort_by = x
    execute_on = timestep_end
    boundary = bottom
  []
[]
