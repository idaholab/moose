[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
  []
[]

[GlobalParams]
  elements = 'Mo Ru Pd'
  output_phases = 'BCCN HCPN'
  output_species = 'HCPN:Pd'
  output_element_potentials = 'Pd'
  output_vapor_pressures = 'gas_ideal:Pd'
  output_element_phases = 'BCCN:Pd'
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
    function = '800*(1-x)+4.3*x'
  []
  [Ru]
    type = FunctionIC
    variable = Ru
    function = '200*(1-x)+4.5*x'
  []
  [Pd]
    type = ConstantIC
    variable = Pd
    value = 1.0e-8
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
