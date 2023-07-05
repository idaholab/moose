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
  output_phases = 'gas_ideal MSFL'
  output_species = 'gas_ideal:Li MSFL:LiF'
  output_element_potentials = 'ALL'
  output_vapor_pressures = 'vp:gas_ideal:LiF'
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
    initial_condition = 750
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

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
