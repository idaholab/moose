# Checking that gravity head is established
# 1phase, constant and large fluid-bulk, constant viscosity, constant permeability
# fully saturated with fully-saturated Kernel
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
    type = PorousFlowFullySaturatedDarcyBase
    variable = pp
    gravity = '-1 0 0'
  []
[]

[Functions]
  [ana_pp]
    type = ParsedFunction
    symbol_names = 'g B p0 rho0'
    symbol_values = '1 1E3 0 1'
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
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 1e3
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
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1 0 0  0 2 0  0 0 3'
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
  file_base = fully_saturated_grav01b
  [csv]
    type = CSV
  []
[]
