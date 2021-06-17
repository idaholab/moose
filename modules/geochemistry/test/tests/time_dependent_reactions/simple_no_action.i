# This example is simple.i but without using an Action
# Simple example of time-dependent reaction path.
# This example involves an HCl solution that is initialized at pH=2, then the pH is controlled via controlled_activity, and finally HCl is titrated into the solution
[GlobalParams]
  point = '0 0 0'
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [u]
    type = Diffusion
    variable = u
  []
[]

[AuxVariables]
  [act_H+]
  []
  [solution_temperature]
  []
  [kg_solvent_H2O]
  []
  [activity_H2O]
  []
  [bulk_moles_H2O]
  []
  [pH]
  []
  [molal_H+]
  []
  [molal_Cl-]
  []
  [molal_HCl]
  []
  [molal_OH-]
  []
  [mg_per_kg_H+]
  []
  [mg_per_kg_Cl-]
  []
  [mg_per_kg_HCl]
  []
  [mg_per_kg_OH-]
  []
  [activity_H+]
  []
  [activity_Cl-]
  []
  [activity_HCl]
  []
  [activity_OH-]
  []
  [bulk_moles_H+]
  []
  [bulk_moles_Cl-]
  []
  [bulk_moles_HCl]
  []
  [bulk_moles_OH-]
  []
[]

[AuxKernels]
  [act_H+]
    type = FunctionAux
    variable = act_H+
    function = '10^(-2 - t)'
    execute_on = timestep_begin
  []
  [solution_temperature]
    type = GeochemistryQuantityAux
    species = 'H+'
    reactor = reactor
    variable = solution_temperature
    quantity = temperature
  []
  [kg_solvent_H2O]
    type = GeochemistryQuantityAux
    species = 'H2O'
    reactor = reactor
    variable = kg_solvent_H2O
    quantity = molal
  []
  [activity_H2O]
    type = GeochemistryQuantityAux
    species = 'H2O'
    reactor = reactor
    variable = activity_H2O
    quantity = activity
  []
  [bulk_moles_H2O]
    type = GeochemistryQuantityAux
    species = 'H2O'
    reactor = reactor
    variable = bulk_moles_H2O
    quantity = bulk_moles
  []
  [pH]
    type = GeochemistryQuantityAux
    species = 'H+'
    reactor = reactor
    variable = pH
    quantity = neglog10a
  []
  [molal_H+]
    type = GeochemistryQuantityAux
    species = 'H+'
    reactor = reactor
    variable = 'molal_H+'
    quantity = molal
  []
  [molal_Cl-]
    type = GeochemistryQuantityAux
    species = 'Cl-'
    reactor = reactor
    variable = 'molal_Cl-'
    quantity = molal
  []
  [molal_HCl]
    type = GeochemistryQuantityAux
    species = 'HCl'
    reactor = reactor
    variable = 'molal_HCl'
    quantity = molal
  []
  [molal_OH-]
    type = GeochemistryQuantityAux
    species = 'OH-'
    reactor = reactor
    variable = 'molal_OH-'
    quantity = molal
  []
  [mg_per_kg_H+]
    type = GeochemistryQuantityAux
    species = 'H+'
    reactor = reactor
    variable = 'mg_per_kg_H+'
    quantity = mg_per_kg
  []
  [mg_per_kg_Cl-]
    type = GeochemistryQuantityAux
    species = 'Cl-'
    reactor = reactor
    variable = 'mg_per_kg_Cl-'
    quantity = mg_per_kg
  []
  [mg_per_kg_HCl]
    type = GeochemistryQuantityAux
    species = 'HCl'
    reactor = reactor
    variable = 'mg_per_kg_HCl'
    quantity = mg_per_kg
  []
  [mg_per_kg_OH-]
    type = GeochemistryQuantityAux
    species = 'OH-'
    reactor = reactor
    variable = 'mg_per_kg_OH-'
    quantity = mg_per_kg
  []
  [activity_H+]
    type = GeochemistryQuantityAux
    species = 'H+'
    reactor = reactor
    variable = 'activity_H+'
    quantity = activity
  []
  [activity_Cl-]
    type = GeochemistryQuantityAux
    species = 'Cl-'
    reactor = reactor
    variable = 'activity_Cl-'
    quantity = activity
  []
  [activity_HCl]
    type = GeochemistryQuantityAux
    species = 'HCl'
    reactor = reactor
    variable = 'activity_HCl'
    quantity = activity
  []
  [activity_OH-]
    type = GeochemistryQuantityAux
    species = 'OH-'
    reactor = reactor
    variable = 'activity_OH-'
    quantity = activity
  []
  [bulk_moles_H+]
    type = GeochemistryQuantityAux
    species = 'H+'
    reactor = reactor
    variable = 'bulk_moles_H+'
    quantity = bulk_moles
  []
  [bulk_moles_Cl-]
    type = GeochemistryQuantityAux
    species = 'Cl-'
    reactor = reactor
    variable = 'bulk_moles_Cl-'
    quantity = bulk_moles
  []
  [bulk_moles_HCl]
    type = GeochemistryQuantityAux
    species = 'HCl'
    reactor = reactor
    variable = 'bulk_moles_HCl'
    quantity = bulk_moles
  []
  [bulk_moles_OH-]
    type = GeochemistryQuantityAux
    species = 'OH-'
    reactor = reactor
    variable = 'bulk_moles_OH-'
    quantity = bulk_moles
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
  [reactor]
    type = GeochemistryTimeDependentReactor
    model_definition = definition
    charge_balance_species = "Cl-"
    constraint_species = "H2O              H+            Cl-"
    constraint_value = "  1.0              -2            1E-2"
    constraint_meaning = "kg_solvent_water log10activity bulk_composition"
    constraint_unit = "   kg               dimensionless moles"
    ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping

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
  [nnn]
    type = NearestNodeNumberUO
  []
[]

[Outputs]
  csv = true
  file_base = simple_out
  [console_output]
    type = GeochemistryConsoleOutput
    geochemistry_reactor = reactor
    nearest_node_number_UO = nnn
    solver_info = true
    execute_on = 'final'
  []
[]
