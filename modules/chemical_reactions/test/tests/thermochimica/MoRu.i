[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
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
    temperature_unit = K
    pressure_unit = atm
    composition_unit = moles
    temperature = 2250
    species_output_unit = mole_fraction
  []
[]

[ICs]
  [Mo]
    type = FunctionIC
    variable = Mo
    function = '0.8*(1-x)+4.3*x'
  []
  [Ru]
    type = FunctionIC
    variable = Ru
    function = '0.2*(1-x)+4.5*x'
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
