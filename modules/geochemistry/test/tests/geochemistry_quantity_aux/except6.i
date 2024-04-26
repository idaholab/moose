#Test for an inappropriate geochemical quantity
[TimeIndependentReactionSolver]
    #Basic configuration for the reaction solver
    model_definition = definition 
    charge_balance_species = "Cl-" 
    constraint_species = "H2O H+ Cl- Fe+++" 
    constraint_value = "  1.0 1.0 1.0 1.0" 
    constraint_meaning = "kg_solvent_water bulk_composition bulk_composition bulk_composition" 
    constraint_unit = "kg moles moles moles"
[]

[UserObjects]
#Definition of the geochemical model
    [definition] 
        type = GeochemicalModelDefinition 
        database_file = "../../database/ferric_hydroxide_sorption.json" 
        basis_species = "H2O H+ Cl- Fe+++" 
        equilibrium_minerals = "Fe(OH)3(ppd)_nosorb"
    []
[]

[AuxVariables]
#Aux variable to hold error or resulting data
    [error]
    []
[]

[AuxKernels]
    #AuxKernel to compute a nonexistent quantity
    [error] 
        type = GeochemistryQuantityAux 
        species = "Fe(OH)3(ppd)_nosorb" 
        reactor = geochemistry_reactor 
        variable = error 
        quantity = "non_existent_quantity" #Triggers the exception
    []
[]
