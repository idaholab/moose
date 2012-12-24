#include "AddCoupledEqSpeciesKernelsAction.h"
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
InputParameters validParams<AddCoupledEqSpeciesKernelsAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<std::vector<NonlinearVariableName> >("primary_species", "The list of primary variables to add");
  params.addRequiredParam<std::vector<std::string> >("eq_reactions", "The list of aqueous equilibrium reactions");
  params.addRequiredParam<std::vector<Real> >("eq_constants", "The list of equilibrium constants for aqueous equilibrium reactions");
  params.addParam<std::string>("pressure","Checks if pressure is a primary variable");

  return params;
}


AddCoupledEqSpeciesKernelsAction::AddCoupledEqSpeciesKernelsAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
AddCoupledEqSpeciesKernelsAction::act()
{
  std::vector<NonlinearVariableName> vars = getParam<std::vector<NonlinearVariableName> >("primary_species");
  std::vector<std::string> reactions = getParam<std::vector<std::string> >("eq_reactions");
  std::vector<Real> keq = getParam<std::vector<Real> >("eq_constants");
  
  std::cout<< "reaction list:" << "\n";
  for (unsigned int i=0; i < reactions.size(); i++)
  {
    std::cout<< reactions[i] << "\n";
  }

  for (unsigned int i=0; i < vars.size(); i++)
  {
    std::cout << "primary species - " << vars[i] << "\n";
    std::vector<bool> primary_participation(reactions.size(), false);
    std::string eq_species;
    
    for (unsigned int j=0; j < reactions.size(); j++)
    {
      std::cout << "\n";
      std::cout << "For reaction " << reactions[j]+":" << "\n";

      std::vector<std::string> tokens;
      // Parsing each reaction
      Parser::tokenize(reactions[j], tokens, 1, "+=");
      
      std::vector<std::string> rxn_vars(tokens.size()-1);
      std::vector<Real> stos(tokens.size()-1);
      std::vector<std::string> coupled_v;

      // Organize participating primary species and corresponding stoichiometrics into separate arrays
      for (unsigned int k=0; k < tokens.size(); k++)
      {
        std::cout << tokens[k] << "\t";
        std::vector<std::string> stos_vars;
        Parser::tokenize(tokens[k], stos_vars, 1, "()");
        if (stos_vars.size() == 2)
        {
          Real coef;
          std::istringstream iss(stos_vars[0]);
          iss >> coef;
          stos[k] = coef;
          rxn_vars[k] = stos_vars[1];
          std::cout << "stochiometric: " << stos[k] << "\t";
          std::cout << "reactant: " << rxn_vars[k] << "\n";
          // Check the participation of primary species
          if (rxn_vars[k] == vars[i]) primary_participation[j] = true;
        }
        else
        {
          eq_species = stos_vars[0];
        }
      }
      // Done parsing, recorded stochiometric and variables into separate arrays
      std::cout << "whether primary present (0 is not): " << primary_participation[j] << "\n";
      std::cout << "equilibrium species: " << eq_species << "\n";
      
      //  Adding the coupled kernels if the primary species participates in this equilibrium reaction
      if (primary_participation[j])
      {
        // Assigning the stochiometrics based on parsing
        Real sto_u;
        std::vector<Real> sto_v;
        Real weight;
        for (unsigned int m=0; m < rxn_vars.size(); m++)
        {
          if (rxn_vars[m] == vars[i])
          {
            weight = stos[m];
            sto_u = stos[m];
            std::cout << "stochio for u: " << sto_u << "\n";
          }
          else
          {
            sto_v.push_back(stos[m]);
            std::cout << "stochio for v: " << sto_v[sto_v.size() - 1] << "\t";
            coupled_v.push_back(rxn_vars[m]);
            std::cout << "coupled variable: " << coupled_v[coupled_v.size() - 1] << "\t";
          }
        }
        
        // Building kernels for equilbirium aqueous species
        InputParameters params_sub = Factory::instance()->getValidParams("CoupledBEEquilibriumSub");
        params_sub.set<NonlinearVariableName>("variable") = vars[i];
        params_sub.set<Real>("weight") = weight;
        params_sub.set<Real>("log_k") = keq[j];
        params_sub.set<Real>("sto_u") = sto_u;
        params_sub.set<std::vector<Real> >("sto_v") = sto_v;
        params_sub.set<std::vector<std::string> >("v") = coupled_v;
        _problem->addKernel("CoupledBEEquilibriumSub", vars[i]+"_"+eq_species+"_sub", params_sub);
        
        std::cout << vars[i]+"_"+eq_species+"_sub" << "\n";
        params_sub.print();
        
        InputParameters params_cd = Factory::instance()->getValidParams("CoupledDiffusionReactionSub");
        params_cd.set<NonlinearVariableName>("variable") = vars[i];
        params_cd.set<Real>("weight") = weight;
        params_cd.set<Real>("log_k") = keq[j];
        params_cd.set<Real>("sto_u") = sto_u;
        params_cd.set<std::vector<Real> >("sto_v") = sto_v;
        params_cd.set<std::vector<std::string> >("v") = coupled_v;
        _problem->addKernel("CoupledDiffusionReactionSub", vars[i]+"_"+eq_species+"_cd", params_cd);
        
        std::cout << vars[i]+"_"+eq_species+"_diff" << "\n";
        params_cd.print();
        
        std::cout << "whether pressure is present" << _pars.isParamValid("pressure") << "\n";
        
        if (_pars.isParamValid("pressure"))
        {
          std::string p;
          p = getParam<std::string>("pressure");
          std::vector<std::string> press(1);
          press[0] = p;
          std::cout << "coupled gradient of p" << press[0] << "\n";
          
          InputParameters params_conv = Factory::instance()->getValidParams("CoupledConvectionReactionSub");
          params_conv.set<NonlinearVariableName>("variable") = vars[i];
          params_conv.set<Real>("weight") = weight;
          params_conv.set<Real>("log_k") = keq[j];
          params_conv.set<Real>("sto_u") = sto_u;
          params_conv.set<std::vector<Real> >("sto_v") = sto_v;
          params_conv.set<std::vector<std::string> >("v") = coupled_v;
          // Pressure is required to be named as "pressure" if it is a primary variable
          params_conv.set<std::vector<std::string> >("p") = press;
          _problem->addKernel("CoupledConvectionReactionSub", vars[i]+"_"+eq_species+"_conv", params_conv);
          
          std::cout << vars[i]+"_"+eq_species+"_conv" << "\n";
          params_conv.print();
        }
      }
      
    }
    std::cout << "\n";
  }
}
