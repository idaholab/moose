# Pressure pulse in 1D with 1 phase, 3 component - transient
# using the PorousFlowFullySaturatedDarcyFlow Kernel
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
  xmin = 0
  xmax = 100
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [pp]
    initial_condition = 2E6
  []
  [f0]
    initial_condition = 0
  []
  [f1]
    initial_condition = 0.2
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
  [flux0]
    type = PorousFlowFullySaturatedDarcyFlow
    variable = pp
    gravity = '0 0 0'
    fluid_component = 0
  []
  [mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = f0
  []
  [flux1]
    type = PorousFlowFullySaturatedDarcyFlow
    variable = f0
    gravity = '0 0 0'
    fluid_component = 1
  []
  [mass2]
    type = PorousFlowMassTimeDerivative
    fluid_component = 2
    variable = f1
  []
  [flux2]
    type = PorousFlowFullySaturatedDarcyFlow
    variable = f1
    gravity = '0 0 0'
    fluid_component = 2
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp f0 f1'
    number_fluid_phases = 1
    number_fluid_components = 3
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2e9
    density0 = 1000
    thermal_expansion = 0
    viscosity = 1e-3
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pp
  []
  [massfrac_nodes]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'f0 f1'
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
    permeability = '1E-15 0 0 0 1E-15 0 0 0 1E-15'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    boundary = left
    preset = false
    value = 3E6
    variable = pp
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -pc_factor_shift_type'
    petsc_options_value = 'bcgs lu 1E-15 1E-10 10000 NONZERO'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1E3
  end_time = 1E4
[]

[Postprocessors]
  [p005]
    type = PointValue
    variable = pp
    point = '5 0 0'
    execute_on = 'initial timestep_end'
  []
  [p015]
    type = PointValue
    variable = pp
    point = '15 0 0'
    execute_on = 'initial timestep_end'
  []
  [p025]
    type = PointValue
    variable = pp
    point = '25 0 0'
    execute_on = 'initial timestep_end'
  []
  [p035]
    type = PointValue
    variable = pp
    point = '35 0 0'
    execute_on = 'initial timestep_end'
  []
  [p045]
    type = PointValue
    variable = pp
    point = '45 0 0'
    execute_on = 'initial timestep_end'
  []
  [p055]
    type = PointValue
    variable = pp
    point = '55 0 0'
    execute_on = 'initial timestep_end'
  []
  [p065]
    type = PointValue
    variable = pp
    point = '65 0 0'
    execute_on = 'initial timestep_end'
  []
  [p075]
    type = PointValue
    variable = pp
    point = '75 0 0'
    execute_on = 'initial timestep_end'
  []
  [p085]
    type = PointValue
    variable = pp
    point = '85 0 0'
    execute_on = 'initial timestep_end'
  []
  [p095]
    type = PointValue
    variable = pp
    point = '95 0 0'
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  file_base = pressure_pulse_1d_3comp_fully_saturated
  print_linear_residuals = false
  csv = true
[]
