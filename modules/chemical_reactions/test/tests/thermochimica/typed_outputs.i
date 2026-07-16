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
      [Phases]
        [hcp_amount]
          phase = HCPN
        []
        [bcc_phase_fraction]
          phase = BCCN
          unit = mole_fraction
        []
        [hcp_phase_fraction]
          phase = HCPN
          unit = mole_fraction
        []
      []
      [Species]
        [hcp_mo_fraction]
          phase = HCPN
          species = Mo
          unit = mole_fraction
        []
        [hcp_mo_moles]
          phase = HCPN
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
      [ElementDistribution]
        [mo_in_hcp]
          phase = HCPN
          element = Mo
          variable = hcp_mo_element_amount
        []
        [mo_in_bcc_fraction]
          phase = BCCN
          element = Mo
          unit = fraction
        []
        [mo_in_hcp_fraction]
          phase = HCPN
          element = Mo
          unit = fraction
        []
      []
    []
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

[Postprocessors]
  [hcp_amount]
    type = NodalVariableValue
    variable = hcp_amount
    nodeid = 1
  []
  [hcp_mo_fraction]
    type = NodalVariableValue
    variable = hcp_mo_fraction
    nodeid = 1
  []
  [bcc_phase_fraction]
    type = NodalVariableValue
    variable = bcc_phase_fraction
    nodeid = 1
  []
  [hcp_phase_fraction]
    type = NodalVariableValue
    variable = hcp_phase_fraction
    nodeid = 1
  []
  [hcp_mo_moles]
    type = NodalVariableValue
    variable = hcp_mo_moles
    nodeid = 1
  []
  [mo_chemical_potential]
    type = NodalVariableValue
    variable = mo_chemical_potential
    nodeid = 1
  []
  [mo_vapor_pressure]
    type = NodalVariableValue
    variable = mo_vapor_pressure
    nodeid = 1
  []
  [hcp_mo_element_amount]
    type = NodalVariableValue
    variable = hcp_mo_element_amount
    nodeid = 1
  []
  [mo_in_bcc_fraction]
    type = NodalVariableValue
    variable = mo_in_bcc_fraction
    nodeid = 1
  []
  [mo_in_hcp_fraction]
    type = NodalVariableValue
    variable = mo_in_hcp_fraction
    nodeid = 1
  []
[]

[Outputs]
  csv = true
[]
