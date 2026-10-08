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
  output_element_potentials = 'O Ti Fe'
  output_vapor_pressures = 'gas_ideal:O2'
[]

[ChemicalComposition]
  [thermo]
    thermodynamic_database = FeTiVO.dat
    temperature_unit = K
    pressure_unit = atm
    composition_unit = moles
    temperature = T
    species_output_unit = mole_fraction
    warm_start = none
  []
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
