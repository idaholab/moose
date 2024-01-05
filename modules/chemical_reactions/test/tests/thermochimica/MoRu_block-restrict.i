[Mesh]
  [two_blocks]
    type = CartesianMeshGenerator
    dim = 2
    dx = '5 10'
    dy = '1'
    ix = '5 5'
    iy = '5'
    subdomain_id = '0 1'
  []
[]

[GlobalParams]
  elements = 'Mo Ru'
  output_phases = 'BCCN HCPN'
  output_element_phases = 'ep:BCCN:Mo'
[]

[ChemicalComposition]
  [thermo]
    block = '0'
    thermofile = Kaye_NobleMetals.dat
    tunit = K
    punit = atm
    munit = moles
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
