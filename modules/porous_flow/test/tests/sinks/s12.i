# The PorousFlowEnthalpy sink adds heat energy corresponding to injecting a 1kg/s/m^2 (flux_function = -1)
# of fluid at pressure 0.5 (given by the AuxVariable p_aux) and the input temperature is 300 (given by the T_in parameter).
# SimpleFluidProperties are used, with density0 = 10, bulk_modulus = 1, thermal_expansion = 0, and cv = 1E-4
# density = 10 * exp(0.5 / 1 + 0) = 16.4872
# internal energy = 1E-4 * 300 = 0.03
# enthalpy = 0.03 + 0.5/16.3872 = 0.0603265
# This is applied over an area of 100, so the total energy flux is 6.03265 J/s.
# This the the rate of change of the heat energy reported by the PorousFlowHeatEnergy Postprocessor

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
  []
[]

[AuxVariables]
  [p_aux]
    initial_condition = 0.5
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
    type = PorousFlowEnergyTimeDerivative
    variable = temp
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 1
    density0 = 10
    thermal_expansion = 0
    cv = 1E-4
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
    porosity = 0.1
  []

  [temperature]
    type = PorousFlowTemperature
    temperature = temp
  []
  [heat]
    type = PorousFlowMatrixInternalEnergy
    specific_heat_capacity = 2
    density = 3
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
    type = PorousFlowEnthalpySink
    variable = temp
    boundary = left
    T_in = 300
    fp = simple_fluid
    flux_function = -1
    porepressure_var = p_aux
  []
[]

[Postprocessors]
  [total_heat_energy]
    type = PorousFlowHeatEnergy
    phase = 0
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
  num_steps = 2
[]

[Outputs]
  file_base = s12
  [csv]
    type = CSV
  []
[]
