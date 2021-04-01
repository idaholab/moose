# Simple example of time-independent problem involving an HCl solution at pH = 2
[GlobalParams]
  point = '0 0 0'
[]

[TimeIndependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = "Cl-"
  constraint_species = "H2O              H+            Cl-"
  constraint_value = "  1.0              -2            1E-2"
  constraint_meaning = "kg_solvent_water log10activity bulk_composition"
  constraint_unit = "   kg               dimensionless moles"
  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping
  execute_console_output_on = initial
  abs_tol = 1E-15
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

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl-"
    piecewise_linear_interpolation = true # to reproduce the GWB result
  []
[]

[Outputs]
  csv = true
[]
