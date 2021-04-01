# Sorption onto FerricHydroxide along with changing pH
[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  swap_out_of_basis = "Fe+++"
  swap_into_basis = "Fe(OH)3(ppd)"
  charge_balance_species = "Cl-"
  constraint_species = "H2O              H+            Na+              Cl-              Fe(OH)3(ppd) >(s)FeOH         >(w)FeOH"
  constraint_value = "  1.0              -4            0.1              0.1              9.3573E-3    4.6786E-5        1.87145E-3"
  constraint_meaning = "kg_solvent_water log10activity bulk_composition bulk_composition free_mineral bulk_composition bulk_composition"
  constraint_unit =    "kg               dimensionless moles            moles            moles        moles            moles"
  controlled_activity_name = "H+"
  controlled_activity_value = set_aH
  ramp_max_ionic_strength_initial = 0 # not needed in this simple problem
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
  abs_tol = 1E-14
  execute_console_output_on = '' # only CSV output needed for this example
[]

[AuxVariables]
  [set_aH]
  []
[]
[AuxKernels]
  [set_aH]
    type = FunctionAux
    variable = set_aH
    function = '10^(-4-t)'
    execute_on = timestep_begin # so the correct value is provided to the reactor
  []
[]

[Postprocessors]
  [pH]
    type = PointValue
    point = '0 0 0'
    variable = 'pH'
  []
  [molal_>wFeOH2+]
    type = PointValue
    point = '0 0 0'
    variable = 'molal_>(w)FeOH2+'
  []
  [molal_>wFeOH]
    type = PointValue
    point = '0 0 0'
    variable = 'molal_>(w)FeOH'
  []
  [molal_>wFeO-]
    type = PointValue
    point = '0 0 0'
    variable = 'molal_>(w)FeO-'
  []
  [molal_>sFeOH2+]
    type = PointValue
    point = '0 0 0'
    variable = 'molal_>(s)FeOH2+'
  []
  [molal_>sFeOH]
    type = PointValue
    point = '0 0 0'
    variable = 'molal_>(s)FeOH'
  []
  [molal_>sFeO-]
    type = PointValue
    point = '0 0 0'
    variable = 'molal_>(s)FeO-'
  []
  [potential]
    type = PointValue
    point = '0 0 0'
    variable = 'surface_potential_Fe(OH)3(ppd)'
  []
[]

[Executioner]
  type = Transient
  start_time = -0.25
  dt = 0.25
  end_time = 8
[]
[Outputs]
  csv = true
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../database/ferric_hydroxide_sorption.json"
    basis_species = "H2O H+ Na+ Cl- Fe+++ >(s)FeOH >(w)FeOH"
    equilibrium_minerals = "Fe(OH)3(ppd)"
    piecewise_linear_interpolation = true # for comparison with GWB
  []
[]

