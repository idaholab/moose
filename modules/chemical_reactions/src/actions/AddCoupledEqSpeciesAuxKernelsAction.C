#include "AddCoupledEqSpeciesAuxKernelsAction.h"
#include "MooseUtils.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseEnum.h"

#include <sstream>
#include <stdexcept>

// libMesh includes
#include "libmesh/libmesh.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/equation_systems.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/explicit_system.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/fe.h"


template<>
InputParameters validParams<AddCoupledEqSpeciesAuxKernelsAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::vector<std::string> >("eq_reactions", "The list of aqueous equilibrium reactions");
  params.addRequiredParam<std::vector<Real> >("eq_constants", "The list of equilibrium constants for aqueous equilibrium reactions");
  params.addParam<std::vector<std::string> >("secondary_species", "The list of aqueous equilibrium species to be output as aux variables");

  return params;
}


AddCoupledEqSpeciesAuxKernelsAction::AddCoupledEqSpeciesAuxKernelsAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
AddCoupledEqSpeciesAuxKernelsAction::act()
{
  std::vector<std::string> reactions = getParam<std::vector<std::string> >("eq_reactions");
  std::vector<Real> keq = getParam<std::vector<Real> >("eq_constants");
  
  if (_pars.isParamValid("secondary_species"))
  {
    std::vector<std::string> secondary_species = getParam<std::vector<std::string> >("secondary_species");
    std::set<std::string> aux_species;
    
    for (unsigned int k=0; k < secondary_species.size(); k++)
    {
      aux_species.insert(secondary_species[k]);
    }
    
    for (unsigned int j=0; j < reactions.size(); j++)
    {
      std::vector<std::string> tokens;
      
      // Parsing each reaction
      MooseUtils::tokenize(reactions[j], tokens, 1, "+=");
      
      std::vector<Real> stos(tokens.size()-1); 
      std::vector<std::string> rxn_vars(tokens.size()-1);
      std::vector<std::string> eq_species(reactions.size());
      
      for (unsigned int k=0; k < tokens.size(); k++)
      {
        std::vector<std::string> stos_vars;
        MooseUtils::tokenize(tokens[k], stos_vars, 1, "()");
        if (stos_vars.size() == 2)
        {
          Real coef;
          std::istringstream iss(stos_vars[0]);
          iss >> coef;
          stos[k] = coef;
          rxn_vars[k] = stos_vars[1];
        }
        else
        {
          eq_species[j] = stos_vars[0];
        }
      }
      // Done parsing, recorded stochiometric and variables into separate arrays
      
      // Adding the AqueousEquilibriumRxnAux auxkernel for the list of eq species read in from the input file
      if (aux_species.find(eq_species[j]) != aux_species.end())
      {
        InputParameters params_eq = _factory.getValidParams("AqueousEquilibriumRxnAux");
        params_eq.set<AuxVariableName>("variable") = eq_species[j];
        params_eq.set<Real>("log_k") = keq[j];
        params_eq.set<std::vector<Real> >("sto_v") = stos;
        params_eq.set<std::vector<std::string> >("v") = rxn_vars;
        _problem->addAuxKernel("AqueousEquilibriumRxnAux", "aux_"+eq_species[j], params_eq);
        
        std::cout << "aux_"+eq_species[j] << "\n";
        params_eq.print();
      }
//       else
//       {
//         mooseError("The secondary species for output doesn't exist in the system!");
//       }
      
    }
  }
}
