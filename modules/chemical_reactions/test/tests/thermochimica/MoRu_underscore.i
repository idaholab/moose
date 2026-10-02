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
  output_species = 'BCCN_Mo HCPN_Mo BCCN_Ru HCPN_Ru'
  output_element_potentials = 'mu_Mo mu_Ru'
  output_vapor_pressures = 'vp_gas_ideal_Mo'
  output_element_phases = 'ep_BCCN_Mo'
[]

[ChemicalComposition]
  [thermo]
    thermofile = Kaye_NobleMetals.dat
    tunit = K
    punit = atm
    munit = moles
    output_name_separator = underscore
    temperature = 2250
    output_species_unit = mole_fraction
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
