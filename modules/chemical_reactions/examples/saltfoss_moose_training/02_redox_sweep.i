# =============================================================================
# SaltFoss MOOSE Training -- Segment 3: Spatial redox
# =============================================================================
# A FLiBe-UF4 salt (Li, Be, U, F) swept from a reducing to an oxidizing
# fluorine mass balance, along the length of a one-dimensional mesh.
# `ChemicalComposition` solves an independent Thermochimica equilibrium at
# every node, producing a continuous redox profile in one solve rather than
# a series of single-point calculations.
#
# `ConstituentFractions` reads the U(III)/U(IV) MQMQA cation fractions on
# MSFL's sublattice directly. On the interface published before this
# redesign, no flat output parameter reaches a sublattice constituent --
# only a whole species or a whole-element amount -- so this oxidation-state
# split is new capability, not merely a renamed parameter.
#
# Run:    <chemical_reactions-opt or combined-opt, this branch> -i 02_redox_sweep.i
# Verified (5-node line sample, x=0..1 <-> F=135.0..135.9 mol):
#   mu_F   -534935.85 -> -479362.62 -> -471243.21 -> -463913.82 -> -452596.32 [J/mol]
#   x_U3   1.137e-02 -> 8.898e-03 -> 6.319e-03 -> 3.737e-03 -> 1.151e-03
#   x_U4   1.729e-06 -> 2.156e-03 -> 4.300e-03 -> 6.427e-03 -> 8.527e-03
# =============================================================================
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 4
  []
[]

[ChemicalComposition]
  [salt]
    elements = 'Li Be U F'
    thermodynamic_database = MSD-TC_V4.1_Fluorides_No_Func.dat
    temperature_unit = K
    pressure_unit = atm
    composition_unit = moles
    temperature = 900

    [Outputs]
      [ElementPotentials]
        [mu_F]
          element = F
        []
      []
      [ConstituentFractions]
        [x_U3]
          phase = MSFL
          sublattice = 1
          constituent = 'U[3+]'
        []
        [x_U4]
          phase = MSFL
          sublattice = 1
          constituent = 'U[CN=VI]'
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
  # F sweeps 135.0 (reducing, UF3-rich) -> 135.9 (oxidizing, UF4-rich)
  [F]
    type = FunctionIC
    variable = F
    function = '135.0 + 0.9*x'
  []
[]

[Problem]
  solve = false
[]
[Executioner]
  type = Steady
[]

[VectorPostprocessors]
  [line]
    type = NodalValueSampler
    variable = 'F mu_F x_U3 x_U4'
    sort_by = x
  []
[]

[Outputs]
  csv = true
[]
