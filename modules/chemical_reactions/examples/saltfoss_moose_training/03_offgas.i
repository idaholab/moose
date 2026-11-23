# =============================================================================
# SaltFoss MOOSE Training -- Segment 4: Off-gassing
# =============================================================================
# A FLiBe-UF4 salt with trace Cs and I (classic volatile fission products),
# at 1200 K and 1e-6 atm -- a pressure representative of a cover-gas or
# off-gas system, where gas_ideal is actually stable (at 1 atm this salt
# does not boil and gas_moles would read zero).
#
# This exact demo surfaced a real bug during preparation of this training:
# `pressure` set inside a named ChemicalComposition subblock was being
# silently overridden by the common block's registered default (1 atm) --
# so this off-gassing calculation returned gas_moles = 0, the wrong answer,
# with no error. Fixed in ChemicalCompositionAction::initializeConfiguration()
# (coupledInput() now calls applyCommonUserSetParameters() instead of
# applyParameters()). Regression test: modules/chemical_reactions/test/
# tests/thermochimica/tests, [pressure_override].
#
# Run:    <chemical_reactions-opt or combined-opt, this branch> -i 03_offgas.i
# Verified: gas_moles = 99.729, matching standalone Thermochimica's
# InputScriptMode run at the same composition and pressure. p_CsI = 1.236e-12 atm.
# =============================================================================
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 1
  []
[]

[ChemicalComposition]
  [salt]
    elements = 'Li Be U F Cs I'
    thermodynamic_database = MSD-TC_V4.1_Fluorides_No_Func.dat
    temperature_unit = K
    pressure_unit = atm
    composition_unit = moles
    temperature = 1200
    pressure = 1.0e-6

    [Outputs]
      [Phases]
        [gas_moles]
          phase = gas_ideal
        []
      []
      [VaporPressures]
        [p_CsI]
          phase = gas_ideal
          species = CsI
        []
        [p_CsF]
          phase = gas_ideal
          species = CsF
        []
        [p_I2]
          phase = gas_ideal
          species = I2
        []
      []
    []
  []
[]

[ICs]
  [Li]
    type = ConstantIC
    variable = Li
    value = 66.0
  []
  [Be]
    type = ConstantIC
    variable = Be
    value = 33.0
  []
  [U]
    type = ConstantIC
    variable = U
    value = 1.0
  []
  [Cs]
    type = ConstantIC
    variable = Cs
    value = 0.02
  []
  [I]
    type = ConstantIC
    variable = I
    value = 0.02
  []
  [F]
    type = ConstantIC
    variable = F
    value = 135.0
  []
[]

[Problem]
  solve = false
[]
[Executioner]
  type = Steady
[]

[Postprocessors]
  [gas_moles]
    type = NodalVariableValue
    variable = gas_moles
    nodeid = 0
  []
  [p_CsI]
    type = NodalVariableValue
    variable = p_CsI
    nodeid = 0
  []
  [p_CsF]
    type = NodalVariableValue
    variable = p_CsF
    nodeid = 0
  []
  [p_I2]
    type = NodalVariableValue
    variable = p_I2
    nodeid = 0
  []
[]

[Outputs]
  csv = true
[]
