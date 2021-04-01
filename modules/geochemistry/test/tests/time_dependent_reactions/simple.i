# Simple example of time-dependent reaction path.
# This example involves an HCl solution that is initialized at pH=2, then the pH is controlled via controlled_activity, and finally HCl is titrated into the solution
[GlobalParams]
  point = '0 0 0'
[]

[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = "Cl-"
  constraint_species = "H2O H+ Cl-"
  constraint_value = "  1.0              -2            1E-2"
  constraint_meaning = "kg_solvent_water log10activity bulk_composition"
  constraint_unit = "   kg               dimensionless moles"
  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping
  execute_console_output_on = '' # only CSV output for this example

# close the system at time = 0
  close_system_at_time = 0

# control pH.  This sets pH = 2 + t (see the act_H+ AuxKernel)
  controlled_activity_name = 'H+'
  controlled_activity_value = 'act_H+'

# remove the constraint on H+ activity at time = 5, when, from the previous time-step, pH = 2 + 4 = 6
  remove_fixed_activity_name = 'H+'
  remove_fixed_activity_time = 5

# add 1E-5 moles of HCl every second of the simulation: this has no impact before time = 5 when the fixed-activity constraint it turned off, but then, molality_H+ ~ 1E-6 + 1E-4 * (t - 4), so
# time, approx_pH
# 5, -log10(1E-4) = 4
# 10, -log10(6E-4) = 3.2
  source_species_names = 'HCl'
  source_species_rates = '1E-4'
[]

[AuxVariables]
  [act_H+]
  []
[]

[AuxKernels]
  [act_H+]
    type = FunctionAux
    variable = act_H+
    function = '10^(-2 - t)'
    execute_on = timestep_begin
  []
[]

[Postprocessors]
  [pH]
    type = PointValue
    variable = 'pH'
  []
  [solvent_mass]
    type = PointValue
    variable = 'kg_solvent_H2O'
  []
  [molal_Cl-]
    type = PointValue
    variable = 'molal_Cl-'
  []
  [mg_per_kg_HCl]
    type = PointValue
    variable = 'mg_per_kg_HCl'
  []
  [activity_OH-]
    type = PointValue
    variable = 'activity_OH-'
  []
  [bulk_H+]
    type = PointValue
    variable = 'bulk_moles_H+'
  []
  [temperature]
    type = PointValue
    variable = 'solution_temperature'
  []
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 10
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl-"
  []
[]

[Outputs]
  csv = true
[]
