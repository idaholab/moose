[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[ChemicalComposition]
  thermofile = Kaye_NobleMetals.dat
  tunit = K
  punit = atm
  munit = moles
  temperature = 2250
  output_species_unit = mole_fraction
  elements = 'Mo Ru'
  output_phases = 'BCCN HCPN'
  output_species = 'BCCN:Mo HCPN:Mo BCCN:Ru HCPN:Ru'
  output_element_potentials = 'mu:Mo mu:Ru'
  output_vapor_pressures = 'vp:gas_ideal:Mo'
  output_element_phases = 'ep:BCCN:Mo'

  chemical_potential_element = Mo
  chemical_potential = -157452.85733323
[]

[ICs]
  [Mo]
    type = FunctionIC
    variable = Mo
    function = '0.5'
  []
  [Ru]
    type = FunctionIC
    variable = Ru
    function = '1'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 1
[]

[Postprocessors]
  [Mo]
    type = ElementAverageValue
    variable = Mo
  []
  [Ru]
    type = ElementAverageValue
    variable = Ru
  []
  [mu:Mo]
    type = ElementAverageValue
    variable = mu:Mo
  []
[]

[Outputs]
  csv = true
[]
