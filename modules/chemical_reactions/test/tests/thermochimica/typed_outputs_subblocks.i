[Mesh]
  [two_blocks]
    type = CartesianMeshGenerator
    dim = 1
    dx = '0.5 0.5'
    ix = '1 1'
    subdomain_id = '0 1'
  []
[]

# Declare the composition variables once so both block-restricted systems use the same state.
[AuxVariables]
  [O]
    family = MONOMIAL
    order = CONSTANT
  []
  [Ti]
    family = MONOMIAL
    order = CONSTANT
  []
  [V]
    family = MONOMIAL
    order = CONSTANT
  []
  [Fe]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[ChemicalComposition]
  elements = 'O Ti V Fe'
  thermodynamic_database = FeTiVO.dat
  temperature_unit = K
  pressure_unit = atm
  composition_unit = moles
  evaluation_location = elemental
  temperature = 2000
  warm_start = none

  [phase_composition]
    block = '0'

    [Outputs]
      [Phases]
        [slag_moles]
          phase = SlagBsoln
        []
      []
      [Species]
        [slag_fe2o3_fraction]
          phase = SlagBsoln
          species = Fe2O3
          unit = mole_fraction
        []
      []
      [ConstituentFractions]
        [slag_fe3_fraction]
          phase = SlagBsoln
          sublattice = 1
          constituent = 'Fe[3+]'
        []
      []
    []
  []

  [system_thermodynamics]
    block = '1'

    [Outputs]
      [ElementPotentials]
        [oxygen_potential]
          element = O
        []
      []
      [VaporPressures]
        [oxygen_vapor_pressure]
          phase = gas_ideal
          species = O2
        []
      []
      [SystemGibbsEnergies]
        [system_gibbs_energy]
        []
      []
      [SystemProperties]
        [system_enthalpy]
          property = enthalpy
        []
      []
    []
  []
[]

[ICs]
  [O]
    type = FunctionIC
    variable = O
    function = '2.0*(1-x)+1.6*x'
  []
  [Ti]
    type = FunctionIC
    variable = Ti
    function = '0.5*(1-x)+0.55*x'
  []
  [V]
    type = FunctionIC
    variable = V
    function = '0.5*(1-x)+0.75*x'
  []
  [Fe]
    type = FunctionIC
    variable = Fe
    function = '0.5*(1-x)+0.25*x'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [slag_moles]
    type = ElementalVariableValue
    variable = slag_moles
    elementid = 0
  []
  [slag_fe2o3_fraction]
    type = ElementalVariableValue
    variable = slag_fe2o3_fraction
    elementid = 0
  []
  [slag_fe3_fraction]
    type = ElementalVariableValue
    variable = slag_fe3_fraction
    elementid = 0
  []
  [oxygen_potential]
    type = ElementalVariableValue
    variable = oxygen_potential
    elementid = 1
  []
  [oxygen_vapor_pressure]
    type = ElementalVariableValue
    variable = oxygen_vapor_pressure
    elementid = 1
  []
  [system_gibbs_energy]
    type = ElementalVariableValue
    variable = system_gibbs_energy
    elementid = 1
  []
  [system_enthalpy]
    type = ElementalVariableValue
    variable = system_enthalpy
    elementid = 1
  []
[]

[Outputs]
  csv = true
[]
