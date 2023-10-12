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
  output_element_potentials = 'mu:Pd'
  output_vapor_pressures = 'vp:gas_ideal:Pd'
  output_element_phases = 'ep:BCCN:Pd'
[]

[ChemicalComposition]
  thermofile = Kaye_NobleMetals.dat
  tunit = K
  punit = atm
  munit = moles
  temperature = 2250
  output_species_unit = mole_fraction
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
