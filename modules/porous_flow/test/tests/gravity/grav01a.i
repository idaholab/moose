# Checking that gravity head is established
# 1phase, vanGenuchten, constant fluid-bulk, constant viscosity, constant permeability, Corey relative perm
# fully saturated
# For better agreement with the analytical solution (ana_pp), just increase nx

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
  xmin = -1
  xmax = 0
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [pp]
    [InitialCondition]
      type = RandomIC
      min = 0
      max = 1
    []
  []
[]

[Kernels]
  [flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pp
    gravity = '-1 0 0'
  []
[]

[Functions]
  [ana_pp]
    type = ParsedFunction
    symbol_names = 'g B p0 rho0'
    symbol_values = '1 1.2 0 1'
    expression = '-B*log(exp(-p0/B)+g*rho0*x/B)' # expected pp at base
  []
[]

[BCs]
  [z]
    type = DirichletBC
    variable = pp
    boundary = right
    value = 0
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
    bulk_modulus = 1.2
    density0 = 1
    viscosity = 1
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
    permeability = '1 0 0  0 2 0  0 0 3'
  []
  [relperm]
    type = PorousFlowRelativePermeabilityCorey
    n = 1
    phase = 0
  []
[]

[Postprocessors]
  [pp_base]
    type = PointValue
    variable = pp
    point = '-1 0 0'
  []
  [pp_analytical]
    type = FunctionValuePostprocessor
    function = ana_pp
    point = '-1 0 0'
  []
  [pp_00]
    type = PointValue
    variable = pp
    point = '0 0 0'
  []
  [pp_01]
    type = PointValue
    variable = pp
    point = '-0.1 0 0'
  []
  [pp_02]
    type = PointValue
    variable = pp
    point = '-0.2 0 0'
  []
  [pp_03]
    type = PointValue
    variable = pp
    point = '-0.3 0 0'
  []
  [pp_04]
    type = PointValue
    variable = pp
    point = '-0.4 0 0'
  []
  [pp_05]
    type = PointValue
    variable = pp
    point = '-0.5 0 0'
  []
  [pp_06]
    type = PointValue
    variable = pp
    point = '-0.6 0 0'
  []
  [pp_07]
    type = PointValue
    variable = pp
    point = '-0.7 0 0'
  []
  [pp_08]
    type = PointValue
    variable = pp
    point = '-0.8 0 0'
  []
  [pp_09]
    type = PointValue
    variable = pp
    point = '-0.9 0 0'
  []
  [pp_10]
    type = PointValue
    variable = pp
    point = '-1 0 0'
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
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = grav01a
  [csv]
    type = CSV
  []
[]
