[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1
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
    excluded_phases = HCPN

    [Outputs]
      [Phases]
        [bcc_amount]
          phase = BCCN
          unit = moles
        []
      []
      [SystemGibbsEnergies]
        [system_gibbs]
        []
      []
    []
  []
[]

[ICs]
  [Mo]
    type = ConstantIC
    variable = Mo
    value = 0.2
  []
  [Ru]
    type = ConstantIC
    variable = Ru
    value = 0.8
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
    nodeid = 1
  []
  [system_gibbs]
    type = NodalVariableValue
    variable = system_gibbs
    nodeid = 1
  []
[]

[Outputs]
  csv = true
[]
