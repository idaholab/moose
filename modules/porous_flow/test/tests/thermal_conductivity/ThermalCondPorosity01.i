# Trivial test of PorousFlowThermalConductivityFromPorosity
# Porosity = 0.1
# Solid thermal conductivity = 3
# Fluid thermal conductivity = 2
# Expected porous medium thermal conductivity = 3 * (1 - 0.1) + 2 * 0.1 = 2.9

[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  zmin = -1
  zmax = 0
  nx = 1
  ny = 1
  nz = 1
  # This test uses ElementalVariableValue postprocessors on specific
  # elements, so element numbering needs to stay unchanged
  allow_renumbering = false
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
  []
[]

[Variables]
  [temp]
    initial_condition = 1
  []
  [pp]
    initial_condition = 0
  []
[]

[Kernels]
  [heat_conduction]
    type = PorousFlowHeatConduction
    variable = temp
  []
  [dummy]
    type = Diffusion
    variable = pp
  []
[]

[BCs]
  [temp]
    type = DirichletBC
    variable = temp
    boundary = 'front back'
    value = 1
  []
  [pp]
    type = DirichletBC
    variable = pp
    boundary = 'front back'
    value = 0
  []
[]

[AuxVariables]
  [lambda_x]
    order = CONSTANT
    family = MONOMIAL
  []
  [lambda_y]
    order = CONSTANT
    family = MONOMIAL
  []
  [lambda_z]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [lambda_x]
    type = MaterialRealTensorValueAux
    property = PorousFlow_thermal_conductivity_qp
    row = 0
    column = 0
    variable = lambda_x
  []
  [lambda_y]
    type = MaterialRealTensorValueAux
    property = PorousFlow_thermal_conductivity_qp
    row = 1
    column = 1
    variable = lambda_y
  []
  [lambda_z]
    type = MaterialRealTensorValueAux
    property = PorousFlow_thermal_conductivity_qp
    row = 2
    column = 2
    variable = lambda_z
  []
[]

[Postprocessors]
  [lambda_x]
    type = ElementalVariableValue
    elementid = 0
    variable = lambda_x
    execute_on = 'timestep_end'
  []
  [lambda_y]
    type = ElementalVariableValue
    elementid = 0
    variable = lambda_y
    execute_on = 'timestep_end'
  []
  [lambda_z]
    type = ElementalVariableValue
    elementid = 0
    variable = lambda_z
    execute_on = 'timestep_end'
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp temp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = temp
  []
  [ppss_qp]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pp
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [porosity_qp]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [lambda]
    type = PorousFlowThermalConductivityFromPorosity
    lambda_s = '3 0 0  0 3 0  0 0 3'
    lambda_f = '2 0 0  0 2 0  0 0 2'
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
  []
[]

[Executioner]
  solve_type = Newton
  type = Steady
[]

[Outputs]
  file_base = ThermalCondPorosity01
  csv = true
  execute_on = 'timestep_end'
[]
