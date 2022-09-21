# 1phase, heat advecting with a moving fluid
# Using the FullySaturated Kernel
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 50
  xmin = 0
  xmax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [temp]
    initial_condition = 200
  []
  [pp]
  []
[]

[ICs]
  [pp]
    type = FunctionIC
    variable = pp
    function = '1-x'
  []
[]

[BCs]
  [pp0]
    type = DirichletBC
    variable = pp
    boundary = left
    value = 1
  []
  [pp1]
    type = DirichletBC
    variable = pp
    boundary = right
    value = 0
  []
  [spit_heat]
    type = DirichletBC
    variable = temp
    boundary = left
    value = 300
  []
  [suck_heat]
    type = DirichletBC
    variable = temp
    boundary = right
    value = 200
  []
[]

[Kernels]
  [mass_dot]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
  [advection]
    type = PorousFlowFullySaturatedDarcyBase
    variable = pp
  []
  [energy_dot]
    type = PorousFlowEnergyTimeDerivative
    variable = temp
  []
  [convection]
    type = PorousFlowFullySaturatedHeatAdvection
    variable = temp
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'temp pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 100
    density0 = 1000
    viscosity = 4.4
    thermal_expansion = 0
    cv = 2
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = temp
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.2
  []
  [rock_heat]
    type = PorousFlowMatrixInternalEnergy
    specific_heat_capacity = 1.0
    density = 125
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1.1 0 0 0 2 0 0 0 3'
  []
  [massfrac]
    type = PorousFlowMassFraction
  []
  [PS]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pp
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'gmres bjacobi 1E-15 1E-10 10000'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 0.01
  end_time = 0.6
[]

[VectorPostprocessors]
  [T]
    type = LineValueSampler
    start_point = '0 0 0'
    end_point = '1 0 0'
    num_points = 51
    sort_by = x
    variable = temp
  []
[]

[Outputs]
  file_base = heat_advection_1d_fully_saturated
  [csv]
    type = CSV
    sync_times = '0.1 0.6'
    sync_only = true
  []
[]
