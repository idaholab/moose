[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1
  []
[]

[ChemicalComposition]
  [thermo]
    elements = 'Fe O'
    thermodynamic_database = FeTiVO.dat
    temperature_unit = K
    pressure_unit = atm
    composition_unit = moles
    temperature = 1600

    [Outputs]
      [Species]
        [slag_fe2o3]
          phase = SlagBsoln
          species = Fe2O3
          unit = moles
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
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
[]
