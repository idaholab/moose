[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 1
  []
[]

[GlobalParams]
  elements = 'F Li Be'
  output_phases = 'all'
  output_species = 'all'
  element_potentials = 'all'
  output_vapor_pressures = 'all'
[]

[ChemicalComposition]
  thermofile = MSTDB_v2_Fluoride.dat
  tunit = K
  punit = atm
  munit = moles
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

[UserObjects]
  [data]
    type = ThermochimicaNodalData
    temperature = 1250
    execute_on = 'INITIAL TIMESTEP_END'
    reinit_requested = false # changes parallel results slightly
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
    thermo_nodal_data_uo = data
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
