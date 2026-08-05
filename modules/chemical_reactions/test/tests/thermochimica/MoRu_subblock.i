[Mesh]
  [two_blocks]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.5 1.0'
    dy = '1.0'
    ix = '5 5'
    iy = '5'
    subdomain_id = '0 1'
  []
[]

[ChemicalComposition]
  temperature_unit = K
  pressure_unit = atm
  composition_unit = moles
  evaluation_location = elemental
  temperature = T
  [block_0]
    block = '0'
    elements = 'Mo Ru'
    thermodynamic_database = Kaye_NobleMetals.dat
    output_phases = 'BCCN HCPN'
    output_species = 'BCCN:Mo'
    evaluation_location = nodal
    species_output_unit = mole_fraction
  []
  [block_1]
    block = '1'
    elements = 'Fe O'
    thermodynamic_database = FeTiVO.dat
    output_phases = 'gas_ideal'
    output_species = 'SlagBsoln:Fe2O3'
    species_output_unit = moles
  []
[]

[ICs]
  [Mo]
    type = FunctionIC
    variable = Mo
    function = '0.8*(1-x)+4.3*x'
    block = '0'
  []
  [Ru]
    type = FunctionIC
    variable = Ru
    function = '0.2*(1-x)+4.5*x'
    block = '0'
  []
  [O]
    type = FunctionIC
    variable = O
    function = '2.0*(1-x)+1.6*x'
    block = '1'
  []
  [Fe]
    type = FunctionIC
    variable = Fe
    function = '0.5*(1-x)+0.25*x'
    block = '1'
  []
[]

[AuxVariables]
  [T]
    type = MooseVariable
  []
[]

[AuxKernels]
  [T]
    type = ParsedAux
    variable = T
    use_xyzt = true
    expression = '1250.0+1000.0*(x/1.5)'
    execute_on = 'INITIAL'
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
