#include "AddCoupledSolidKinSpeciesKernelsAction.h"
#include "Parser.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseEnum.h"

#include <sstream>
#include <stdexcept>

// libMesh includes
#include "libmesh.h"
#include "exodusII_io.h"
#include "equation_systems.h"
#include "nonlinear_implicit_system.h"
#include "explicit_system.h"
#include "string_to_enum.h"
#include "fe.h"


template<>
InputParameters validParams<AddCoupledSolidKinSpeciesKernelsAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::vector<NonlinearVariableName> >("primary_species", "The list of primary species to add");
  params.addRequiredParam<std::vector<std::string> >("kin_reactions", "The list of solid kinetic reactions");
  
  return params;
}


AddCoupledSolidKinSpeciesKernelsAction::AddCoupledSolidKinSpeciesKernelsAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
AddCoupledSolidKinSpeciesKernelsAction::act()
{
  std::vector<NonlinearVariableName> vars = getParam<std::vector<NonlinearVariableName> >("primary_species");
  std::vector<std::string> reactions = getParam<std::vector<std::string> >("kin_reactions");
  
  std::cout<< "Solid kinetic reaction list:" << "\n";
  for (unsigned int i=0; i < reactions.size(); i++)
  {
    std::cout<< reactions[i] << "\n";
  }

  for (unsigned int i=0; i < vars.size(); i++)
  {
    std::cout << "primary species - " << vars[i] << "\n";
    std::vector<bool> primary_participation(reactions.size(), false);
    std::vector<std::string> solid_kin_species(reactions.size());
    std::vector<Real> weight;
    
    for (unsigned int j=0; j < reactions.size(); j++)
    {
      std::vector<std::string> tokens;

      // Parsing each reaction
      Parser::tokenize(reactions[j], tokens, 1, "+=");
      
      std::vector<Real> stos(tokens.size()-1);
      std::vector<std::string> rxn_vars(tokens.size()-1);
      
      for (unsigned int k=0; k < tokens.size(); k++)
      {
        std::cout << tokens[k] << "\t";
        std::vector<std::string> stos_vars;
        Parser::tokenize(tokens[k], stos_vars, 1, "()");
        if(stos_vars.size() == 2)
        {
          Real coef;
          std::istringstream iss(stos_vars[0]);
          iss >> coef;
          stos[k] = coef;
          rxn_vars[k] = stos_vars[1];
          std::cout << "stochiometric: " << stos[k] << "\t";
          std::cout << "reactant: " << rxn_vars[k] << "\n";
          // Check the participation of primary species
          if(rxn_vars[k] == vars[i]) primary_participation[j] = true;
        }
        else
        {
          solid_kin_species[j] = stos_vars[0];
        }
      }
      // Done parsing, recorded stochiometric and variables into separate arrays
      std::cout << "whether primary present (0 is not): " << primary_participation[j] << "\n";
      

      if(primary_participation[j])
      {
        // Assigning the stochiometrics based on parsing
        for (unsigned int m=0; m < rxn_vars.size(); m++)
        {
          if(rxn_vars[m] == vars[i])
          {
            weight.push_back(stos[m]);
            std::cout << "weight for " << rxn_vars[m] <<" : " << weight[weight.size()-1] << "\n";
          }
        }
        std::cout << "solid kinetic species: " << solid_kin_species[j] << "\n";

        std::vector<std::string> coupled_var(1);
        coupled_var[0] = solid_kin_species[j];
        
        // Building kernels for solid kinetic species
        InputParameters params_kin = _factory.getValidParams("CoupledBEKinetic");
        params_kin.set<NonlinearVariableName>("variable") = vars[i];
        params_kin.set<std::vector<Real> >("weight") = weight;
        params_kin.set<std::vector<std::string> >("v") = coupled_var;
        _problem->addKernel("CoupledBEKinetic", vars[i]+"_"+solid_kin_species[j]+"_kin", params_kin);
        
        std::cout << vars[i]+"_"+solid_kin_species[j]+"_kin" << "\n";
        params_kin.print();
      }
    }
    std::cout << "\n";
  }
}
