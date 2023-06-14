[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
  []
[]

[GlobalParams]
  elements = 'F Li Be'
  output_phases = 'gas_ideal MSFL'
  output_species = 'gas_ideal:Li'
  # element_potentials = 'ALL'
  # output_vapor_pressures = 'ALL'
[]

[ChemicalComposition]
  thermofile = MSTDB_v2_Fluoride.dat
  tunit = K
  punit = atm
  munit = moles
  temperature = T
  reinit_requested = false
  user_object_name = Thermochimica
[]

[Variables]
  [T]
    type = MooseVariable
    initial_condition = 2250
  []
[]


[ICs]
  [Li]
    type = FunctionIC
    variable = Li
    function = '0.8*(1-x)+4.3*x'
  []
  [Be]
    type = FunctionIC
    variable = Be
    function = '0.2*(1-x)+4.5*x'
  []
  [F]
    type = FunctionIC
    variable = F
    function = '1.201*(1-x)+13.301*x'
  []
[]

[AuxVariables]
  [n]
  []
[]

[AuxKernels]
  [thermochimica]
    type = ThermochimicaAux
    variable = n
    thermo_nodal_data_uo = Thermochimica
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

# [VectorPostprocessors]
#   [NodalData]
#     type = NodalValueSampler
#     variable = 'Mo Ru BCCN:Mo HCPN:Mo BCCN:Ru HCPN:Ru cp:Mo cp:Ru'
#     execute_on = TIMESTEP_END
#     sort_by = id
#   []
# []

# [Debug]
#   show_execution_order = ALWAYS
#   show_actions = true
#   show_action_dependencies = true
# []

[Outputs]
  exodus = true
[]
