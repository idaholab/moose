#Extract molality_per_kg for Cl-
[TimeIndependentReactionSolver]
    model_definition = definition
    charge_balance_species = "Na+"
    constraint_species = "H2O Na+ Cl-"
    constraint_value = "1.0 1.0E-3 1.0E-3"
    constraint_meaning = "kg_solvent_water free_concentration free_concentration"
    constraint_unit = "kg molal molal"
    ramp_max_ionic_strength_initial = 0
[]

[UserObjects]
    [definition]
        type = GeochemicalModelDefinition
        database_file = "../../database/saline_water.json"  # Assuming a different contextually appropriate database
        basis_species = "H2O Na+ Cl-"
    []
[]

[AuxVariables]
    [cl_molality]
    []
[]

[AuxKernels]
    [cl_molality]
        type = GeochemistryQuantityAux
        species = 'Cl-'
        reactor = geochemistry_reactor
        variable = cl_molality
        quantity = molality_per_kg
    []
[]

[Postprocessors]
    [molality_value]
        type = PointValue
        point = '0 0 0'
        variable = cl_molality
    []
    [molality_from_action]
        type = PointValue
        point = '0 0 0'
        variable = "molality_per_kg_Cl-"
    []
[]

[Outputs]
    csv = true
[]
