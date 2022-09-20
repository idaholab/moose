# Tests the Jacobian of PorousFlowEnthalpySink when pore pressure is specified

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[GlobalParams]
  PorousFlowDictator = dictator
  at_nodes = true
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp temp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureConst
    pc = 0.1
  []
[]

[Variables]
  [pp]
    initial_condition = 1
  []
  [temp]
    initial_condition = 2
  []
[]

[Kernels]
  [mass0]
    type = TimeDerivative
    variable = pp
  []

  [heat_conduction]
    type = TimeDerivative
    variable = temp
  []
[]

[FluidProperties]
  [simple_fluid]
    type = IdealGasFluidProperties
  []
[]

[Materials]
  [ppss]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pp
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []

  [temperature]
    type = PorousFlowTemperature
    temperature = temp
  []
[]

[BCs]
  [left]
    type = PorousFlowEnthalpySink
    variable = temp
    boundary = left
    fluid_phase = 0
    T_in = 300
    fp = simple_fluid
    flux_function = -23
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 0.1
  num_steps = 1
  nl_rel_tol = 1E-12
  nl_abs_tol = 1E-12

  petsc_options_iname = '-snes_test_err'
  petsc_options_value = '1e-2'
[]
