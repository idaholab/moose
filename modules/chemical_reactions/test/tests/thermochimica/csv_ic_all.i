[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
  []
[]

[GlobalParams]
  elements = 'ALL'
  output_phases = 'ALL'
  output_species = 'ALL'
  output_element_potentials = 'ALL'
  output_vapor_pressures = 'ALL'
  output_element_phases = 'ALL'
[]

[ChemicalComposition]
  tunit = K
  punit = atm
  munit = moles
  output_species_unit = moles
  [thermo]
    thermofile = Kaye_NobleMetals.dat
    initial_values = ic_all.csv
    temperature = 2250
    reinitialization_type = nodal
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
