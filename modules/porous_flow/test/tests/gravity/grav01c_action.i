# Checking that gravity head is established
# using the Unsaturated Action
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
  block = 0
[]

[Variables]
  [pp]
    [InitialCondition]
      type = RandomIC
      min = -1
      max = 1
    []
  []
[]

[FluidProperties]
  [the_simple_fluid]
    type = SimpleFluidProperties
    thermal_expansion = 0.0
    bulk_modulus = 2.0
    viscosity = 1.0
    density0 = 1.0
  []
[]

[PorousFlowUnsaturated]
  add_saturation_aux = false
  add_darcy_aux = false
  porepressure = pp
  gravity = '-1 0 0'
  fp = the_simple_fluid
  van_genuchten_alpha = 1.0
  van_genuchten_m = 0.5
  relative_permeability_type = Corey
  relative_permeability_exponent = 1.0
[]

[Functions]
  [ana_pp]
    type = ParsedFunction
    symbol_names = 'g B p0 rho0'
    symbol_values = '1 2 -1 1'
    expression = '-B*log(exp(-p0/B)+g*rho0*x/B)' # expected pp at base
  []
[]

[BCs]
  [z]
    type = DirichletBC
    variable = pp
    boundary = right
    value = -1
  []
[]

[Materials]
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
  file_base = grav01c_action
  exodus = true
  [csv]
    type = CSV
  []
[]
