[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1
  []
[]

[ChemicalComposition]
  [thermo]
    elements = 'Fe V Ti O'
    thermodynamic_database = FeTiVO.dat
    temperature_unit = K
    pressure_unit = atm
    composition_unit = moles
    temperature = 2000

    [Outputs]
      [Species]
        [slag_fe2o3]
          phase = SlagBsoln
          species = Fe2O3
          unit = moles
        []
      []
      [ChemicalPotentials]
        [slag_quadruplet_potential]
          phase = SlagBsoln
          quadruplet = 'Fe[3+]-Fe[3+]-O-O'
        []
        [slag_endmember_potential]
          phase = SlagBsoln
          endmember = Fe2O3
        []
        [slag_pair_potential]
          phase = SlagBsoln
          pair = Fe2O3
        []
      []
      [ConstituentFractions]
        [slag_fe3_fraction]
          phase = SlagBsoln
          sublattice = 1
          constituent = 'Fe[3+]'
        []
        [slag_oxygen_fraction]
          phase = SlagBsoln
          sublattice = 2
          constituent = O
        []
        [rutile_vacancy_fraction]
          phase = Rutilesoln
          sublattice = 2
          constituent = Va
        []
      []
    []
  []
[]

[ICs]
  [Fe]
    type = ConstantIC
    variable = Fe
    value = 0.5
  []
  [O]
    type = ConstantIC
    variable = O
    value = 2
  []
  [V]
    type = ConstantIC
    variable = V
    value = 0.5
  []
  [Ti]
    type = ConstantIC
    variable = Ti
    value = 0.5
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [slag_fe3_fraction]
    type = NodalVariableValue
    variable = slag_fe3_fraction
    nodeid = 1
  []
  [slag_oxygen_fraction]
    type = NodalVariableValue
    variable = slag_oxygen_fraction
    nodeid = 1
  []
  [rutile_vacancy_fraction]
    type = NodalVariableValue
    variable = rutile_vacancy_fraction
    nodeid = 1
  []
  [slag_quadruplet_potential]
    type = NodalVariableValue
    variable = slag_quadruplet_potential
    nodeid = 1
  []
  [slag_endmember_potential]
    type = NodalVariableValue
    variable = slag_endmember_potential
    nodeid = 1
  []
  [slag_pair_potential]
    type = NodalVariableValue
    variable = slag_pair_potential
    nodeid = 1
  []
[]

[Outputs]
  csv = true
[]
