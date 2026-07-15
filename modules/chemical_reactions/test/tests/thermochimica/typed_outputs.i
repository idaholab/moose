[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 2
  []
[]

[ChemicalComposition]
  [thermo]
    elements = 'Mo Ru'
    thermodynamic_database = Kaye_NobleMetals.dat
    temperature_unit = K
    pressure_unit = atm
    composition_unit = moles
    temperature = 2250

    [Outputs]
      [PhaseAmounts]
        [bcc_amount]
          phase = BCCN
        []
      []
      [SpeciesAmounts]
        [bcc_mo_fraction]
          phase = BCCN
          species = Mo
          unit = mole_fraction
        []
        [bcc_mo_moles]
          phase = BCCN
          species = Mo
          unit = moles
        []
      []
      [ElementPotentials]
        [mo_chemical_potential]
          element = Mo
        []
      []
      [VaporPressures]
        [mo_vapor_pressure]
          phase = gas_ideal
          species = Mo
        []
      []
      [ElementsInPhases]
        [mo_in_bcc]
          phase = BCCN
          element = Mo
          variable = bcc_mo_element_amount
        []
      []
    []
  []
[]

[ICs]
  [Mo]
    type = ConstantIC
    variable = Mo
    value = 0.8
  []
  [Ru]
    type = ConstantIC
    variable = Ru
    value = 0.2
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [bcc_amount]
    type = NodalVariableValue
    variable = bcc_amount
    nodeid = 0
  []
  [bcc_mo_fraction]
    type = NodalVariableValue
    variable = bcc_mo_fraction
    nodeid = 0
  []
  [bcc_mo_moles]
    type = NodalVariableValue
    variable = bcc_mo_moles
    nodeid = 0
  []
  [mo_chemical_potential]
    type = NodalVariableValue
    variable = mo_chemical_potential
    nodeid = 0
  []
  [mo_vapor_pressure]
    type = NodalVariableValue
    variable = mo_vapor_pressure
    nodeid = 0
  []
  [bcc_mo_element_amount]
    type = NodalVariableValue
    variable = bcc_mo_element_amount
    nodeid = 0
  []
[]

[Outputs]
  csv = true
[]
