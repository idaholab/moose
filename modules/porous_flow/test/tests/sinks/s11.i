# Test PorousFlowEnthalpySink boundary condition

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 2
  nz = 2
  xmin = 0
  xmax = 10
  ymin = 0
  ymax = 10
  zmin = 0
  zmax = 10
[]

[GlobalParams]
  PorousFlowDictator = dictator
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
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []

  [heat_conduction]
    type = TimeDerivative
    variable = temp
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 1
    density0 = 10
    thermal_expansion = 0
    viscosity = 11
  []
[]

[Materials]
  [ppss]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pp
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
    porosity = 0.125
  []

  [temperature]
    type = PorousFlowTemperature
    temperature = temp
  []
[]

[BCs]
  [left_p]
    type = PorousFlowSink
    variable = pp
    boundary = left
    flux_function = -1
  []
  [left_T]
    # Note, there is no `fluid_phase` or `porepressure_var` prescribed, since they are passed in from the `tests` file
    type = PorousFlowEnthalpySink
    variable = temp
    boundary = left
    T_in = 300
    fp = simple_fluid
    flux_function = -1
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
  dt = 0.25
  end_time = 1
  nl_rel_tol = 1E-12
  nl_abs_tol = 1E-12
[]

[Outputs]
  file_base = s11
  [exodus]
    type = Exodus
    execute_on = 'initial final'
  []
[]
