[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
  [block1]
    type = SubdomainBoundingBoxGenerator
    block_id = 0
    bottom_left = '0 0 0'
    top_right = '0.5 1 0'
    input = generated_mesh
  []
  [block2]
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
    input = block1
  []
[]

[ChemicalComposition]
  tunit = K
  punit = atm
  munit = moles
  temperature = T
  [block_0]
    block = '0'
    prefix = salt
    elements = 'Mo Ru'
    thermofile = Kaye_NobleMetals.dat
    output_phases = 'BCCN HCPN'
    output_species = 'BCCN:Mo'
    output_species_unit = mole_fraction
    uo_name = Thermochimica_salt
  []
  [block_1]
    block = '1'
    prefix = metal
    elements = 'Mo Ru'
    thermofile = Kaye_NobleMetals.dat
    output_phases = 'BCCN HCPN'
    output_species = 'BCCN:Mo'
    output_species_unit = mole_fraction
    uo_name = Thermochimica_metal
  []
[]
[AuxVariables]
  [salt_Mo]
  []
  [salt_Ru]
  []
  [metal_Mo]
  []
  [metal_Ru]
  []
[]
[ICs]
  [salt_Mo]
    type = FunctionIC
    variable = salt_Mo
    function = '0.8'
  []
  [salt_Ru]
    type = FunctionIC
    variable = salt_Ru
    function = '0.2'
  []
  [metal_Mo]
    type = FunctionIC
    variable = metal_Mo
    function = '0.5'
  []
  [metal_Ru]
    type = FunctionIC
    variable = metal_Ru
    function = '0.3'
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
    expression = '1250.0'
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
