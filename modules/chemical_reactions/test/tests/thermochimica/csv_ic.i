[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
  []
[]

[GlobalParams]
  elements = 'Mo Ru'
  output_phases = 'BCCN HCPN'
  output_species = 'BCCN:Mo HCPN:Mo BCCN:Ru HCPN:Ru'
  output_element_potentials = 'Mo Ru'
  output_vapor_pressures = 'gas_ideal:Mo'
  output_element_phases = 'BCCN:Mo'
[]

[ChemicalComposition]
  [thermo]
    thermodynamic_database = Kaye_NobleMetals.dat
    initial_composition_file = ic.csv
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
