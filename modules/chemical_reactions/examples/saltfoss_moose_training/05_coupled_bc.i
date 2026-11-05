# =============================================================================
# SaltFoss MOOSE Training -- Segment 6: Coupling out
# =============================================================================
# The redox/corrosion hand-off: a Thermochimica equilibrium output used
# directly as a boundary condition for a transport equation. `mu_F` from
# 02_redox_sweep.i's fluorine chemical potential profile drives the flux on
# a toy "corrosion marker" diffusion field -- a real [Variables] field,
# solved with Newton, not a postprocessor printed off to the side. `mu_F` is
# a genuine coupled variable: any Kernel, BC, or Material can read it like
# any other MOOSE variable.
#
# marker solves Diffusion with marker=0 pinned on the left and a Neumann
# flux on the right proportional to the local fluorine potential (mu_F,
# more negative = more reducing) -- the redox state sets how fast the
# marker field is driven in from that boundary. A real corrosion model
# would couple a chromium dissolution rate through the same mechanism.
#
# Run:    <chemical_reactions-opt or combined-opt, this branch> -i 05_coupled_bc.i
# Verified: "Solve Converged!", marker_right = -0.4526.
# =============================================================================
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 8
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
  # Same reducing -> oxidizing sweep as Segment 3
  [F]
    type = FunctionIC
    variable = F
    function = '135.0 + 0.9*x'
  []
[]

[Variables]
  [marker]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = marker
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = marker
    boundary = left
    value = 0
  []
  # The Thermochimica equilibrium output drives a real FE boundary flux.
  [right]
    type = CoupledVarNeumannBC
    variable = marker
    boundary = right
    v = mu_F
    coef = 1e-6
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Postprocessors]
  [marker_right]
    type = SideAverageValue
    variable = marker
    boundary = right
  []
[]

[Outputs]
  csv = true
[]
