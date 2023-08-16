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
  output_element_potentials = 'mu:Mo mu:Ru'
  output_vapor_pressures = 'vp:gas_ideal:Mo'
  output_element_phases = 'ep:BCCN:Mo'
[]

[ChemicalComposition]
  thermofile = Kaye_NobleMetals.dat
  initial_values = ic.csv
  tunit = K
  punit = atm
  munit = moles
  temperature = 2250
  reinitialization_type = nodal
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
