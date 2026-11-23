# =============================================================================
# SaltFoss MOOSE Training -- Segment 5: Making it affordable
# =============================================================================
# The same redox sweep as 02_redox_sweep.i, on a forty-element mesh --
# forty-one independent equilibrium solves in one run. `warm_start`
# controls the Thermochimica worker's initial guess at each node:
#   previous_solve     -- reuse the result from the preceding solve on this
#                          worker (default; usually fastest for a smooth sweep)
#   previous_timestep   -- reuse this same node's own result from the last
#                          timestep (useful when nodes are visited out of order)
#   none                -- always solve from scratch
# `batch_size` groups multiple nodes into one request to the Thermochimica
# worker process; `report_performance = true` prints exact counters --
# states solved, batches sent, warm starts used, worker solve time.
#
# Run twice, comparing wall time:
#   <chemical_reactions-opt or combined-opt, this branch> -i 04_performance.i
#   <... > -i 04_performance.i ChemicalComposition/salt/warm_start=none
# =============================================================================
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 40
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
    warm_start = previous_solve
    report_performance = true

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

[Outputs]
  csv = true
[]
