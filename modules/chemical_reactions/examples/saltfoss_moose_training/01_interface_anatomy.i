# =============================================================================
# SaltFoss MOOSE Training -- Segment 2 (0:08-0:20): Interface anatomy, typed form
# =============================================================================
# Ported from the SaltFoss MOOSE training's current-interface anatomy example
# (10_current_syntax.i), onto the tc_api interface redesign. Same Mo-Ru
# noble-metal binary and the same five published-interface output families,
# rewritten as typed [Outputs] leaves:
#   output_phases              -> [Phases]
#   output_species              -> [Species]
#   output_element_potentials   -> [ElementPotentials]
#   output_vapor_pressures      -> [VaporPressures]
#   output_element_phases       -> [ElementDistribution]
#
# The published interface encodes the selector into the variable name --
# `mu:Mo`, `BCCN:Mo`, `vp:gas_ideal:Mo`, `ep:BCCN:Mo`. Here every leaf block
# name IS the variable name, chosen by the person writing the input file.
#
# Run:    <chemical_reactions-opt or combined-opt, this branch> -i 10_interface_anatomy_typed.i
# =============================================================================
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
        [bcc_amount]
          phase = BCCN
        []
        [hcp_amount]
          phase = HCPN
        []
      []
      [Species]
        [bcc_mo_fraction]
          phase = BCCN
          species = Mo
          unit = mole_fraction
        []
        [hcp_mo_fraction]
          phase = HCPN
          species = Mo
          unit = mole_fraction
        []
      []
      [ElementPotentials]
        [mo_potential]
          element = Mo
        []
        [ru_potential]
          element = Ru
        []
      []
      [VaporPressures]
        [mo_vapor_pressure]
          phase = gas_ideal
          species = Mo
        []
      []
      [ElementDistribution]
        [mo_in_bcc]
          phase = BCCN
          element = Mo
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

[Outputs]
  exodus = true
[]
