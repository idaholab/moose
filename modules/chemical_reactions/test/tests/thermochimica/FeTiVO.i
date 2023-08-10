[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
  []
[]

[GlobalParams]
  elements = 'O Ti V Fe'
  output_phases = 'gas_ideal SlagBsoln Hemasoln'
  output_species = 'gas_ideal:O2 SlagBsoln:Fe2O3'
  output_element_potentials = 'mu:O mu:Ti mu:Fe'
  output_vapor_pressures = 'vp:gas_ideal:O2'
[]

[ChemicalComposition]
  thermofile = FeTiVO.dat
  tunit = K
  punit = atm
  munit = moles
  temperature = T
  uo_name = Thermochimica
  output_species_unit = mole_fraction
  reinitialization_type = none
[]

[Variables]
  [T]
    type = MooseVariable
    initial_condition = 2000
  []
[]

[ICs]
  [O]
    type = FunctionIC
    variable = O
    function = '2.0*(1-x)+1.6*x'
  []
  [Ti]
    type = FunctionIC
    variable = Ti
    function = '0.5*(1-x)+0.55*x'
  []
  [V]
    type = FunctionIC
    variable = V
    function = '0.5*(1-x)+0.75*x'
  []
  [Fe]
    type = FunctionIC
    variable = Fe
    function = '0.5*(1-x)+0.25*x'
  []
[]

[Problem]
  solve = false
[]

[VectorPostprocessors]
  [Fe2O3]
    type = NodalValueSampler
    variable = SlagBsoln:Fe2O3
    sort_by = x
  []
[]

[Executioner]
  type = Steady
[]
