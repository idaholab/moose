# checking radioactive decay
# 1phase, 1component, constant porosity
#
# Note that we don't get mass = mass0 * exp(-Lambda * t)
# because of the time discretisation.  We are solving
# the equation
# (mass - mass0)/dt = -Lambda * mass
# which has the solution
# mass = mass0/(1 + Lambda * dt)
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 3
  xmin = -1
  xmax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [pp]
  []
[]

[ICs]
  [pinit]
    type = FunctionIC
    function = 10
    variable = pp
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
  [decay]
    type = PorousFlowMassRadioactiveDecay
    fluid_component = 0
    variable = pp
    decay_rate = 2.0
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
    alpha = 1
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 1
    density0 = 1
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
[]

[Postprocessors]
  [total_mass]
    type = PorousFlowFluidMass
    execute_on = 'timestep_end'
  []
  [total_mass0]
    type = PorousFlowFluidMass
    execute_on = 'timestep_begin'
  []
  [should_be_zero]
    type = FunctionValuePostprocessor
    function = should_be_0
  []
[]

[Functions]
  [should_be_0]
    type = ParsedFunction
    symbol_names = 'm0 m rate dt'
    symbol_values = 'total_mass0 total_mass 2.0 1'
    expression = 'm-m0/(1.0+rate*dt)'
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-10 1E-10 10000'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  num_steps = 1
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = radioactive_decay01
  csv = true
[]
