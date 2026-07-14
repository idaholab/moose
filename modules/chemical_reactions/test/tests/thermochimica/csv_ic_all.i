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
  temperature_unit = K
  pressure_unit = atm
  composition_unit = moles
  species_output_unit = moles
  [thermo]
    thermodynamic_database = Kaye_NobleMetals.dat
    initial_composition_file = ic_all.csv
    temperature = 2250
    warm_start = previous_solve
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
