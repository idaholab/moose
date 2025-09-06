//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AqueousReactionKinetics.h"

// Register the actions for the objects actually used
registerMooseAction("ChemicalReactionsApp", AqueousReactionKinetics, "add_kernel");
registerMooseAction("ChemicalReactionsApp", AqueousReactionKinetics, "add_aux_kernel");
registerReactionKineticsPhysicsBaseTasks("ChemicalReactionsApp", AqueousReactionKinetics);

InputParameters
AqueousReactionKinetics::validParams()
{
  InputParameters params = ReactionKineticsPhysicsBase::validParams();
  params.addClassDescription("Forms the equations for the chemical reaction networks in a fluid"
                             " medium using a continuous Galerkin finite element discretization.");

  // Rename parameters to match the previously existing actions for AqueousEquilibrium ReactionNetwork
  params.renameParam("solver_variables", "primary_species", "The list of primary species to add");
  params.renameParam("auxiliary_variables", "secondary_species", "The list of secondary species to add");
  params.renameParam("variable_order", "order", "Order of both the primary and secondary species variables");
  params.renameParam("equation_scaling", "scaling", "");
  params.renameParam("reactions", "reactions", "The list of equilibrium reactions occuring in the fluid");

  // To preserve the legacy option to perform Darcy-advection with AqueousEquilibrium ReactionNetwork
  params.addParam<bool>("add_darcy_advection_term",
      false,
      "Whether to add an advection term using the Darcy equation to compute the advecting velocity");
  params.addParam<std::vector<VariableName>>("pressure", {}, "Pressure variable (for Darcy advection)");
  params.addParam<RealVectorValue>("gravity", "Gravity vector (for Darcy advection)");

  return params;
}

AqueousReactionKinetics::AqueousReactionKinetics(const InputParameters & parameters)
  : ReactionKineticsPhysicsBase(parameters),
    _pressure_var(getParam<std::vector<VariableName>>("pressure")),
    _gravity(getParam<RealVectorValue>("gravity"))
{
  // Further parse the reactions
  _stos.push_back(local_stos);
  _solver_species_involved.push_back(local_species_list);

  // For each primary/solver species, we examine the reactions to get the coefficients
  // and various constants
  for (unsigned int i = 0; i < _solver_species.size(); ++i)
  {
    _sto_u[i].resize(_num_reactions, 0.0);
    _sto_v[i].resize(_num_reactions);
    _coupled_v[i].resize(_num_reactions);
    _weights[i].resize(_num_reactions, 0.0);

    _primary_participation[i].resize(_num_reactions, false);
    for (unsigned int j = 0; j < _num_reactions; ++j)
    {
      for (unsigned int k = 0; k < _solver_species_involved[j].size(); ++k)
        if (_solver_species_involved[j][k] == _solver_species[i])
          _primary_participation[i][j] = true;

      if (_primary_participation[i][j])
      {
        for (unsigned int k = 0; k < _solver_species_involved[j].size(); ++k)
        {
          if (_solver_species_involved[j][k] == _solver_species[i])
          {
            _sto_u[i][j] = _stos[j][k];
            _weights[i][j] = _stos[j][k];
          }
          else
          {
            _sto_v[i][j].push_back(_stos[j][k]);
            _coupled_v[i][j].push_back(_solver_species_involved[j][k]);
          }
        }
      }
    }
  }

  // Parameter checks
  checkSecondParamSetOnlyIfFirstOneTrue("add_darcy_advection_term", "pressure");
  checkSecondParamSetOnlyIfFirstOneTrue("add_darcy_advection_term", "gravity");
}

void
AqueousReactionKinetics::addFEKernels()
{
  // Add Kernels for each primary species
  // Note that the equations are on a per-species basis!
  // So equations are organized differently than reactions
  for (const auto i : index_range(_solver_species))
  {
    for (const auto j : make_range(_num_reactions))
    {
      if (_primary_participation[i][j])
      {
        {
          InputParameters params_sub = _factory.getValidParams("CoupledBEEquilibriumSub");
          assignBlocks(params_sub, _blocks);
          params_sub.set<NonlinearVariableName>("variable") = _solver_species[i];
          params_sub.set<Real>("weight") = _weights[i][j];
          params_sub.defaultCoupledValue("log_k", _eq_const[j]);
          params_sub.set<Real>("sto_u") = _sto_u[i][j];
          params_sub.set<std::vector<Real>>("sto_v") = _sto_v[i][j];
          params_sub.set<std::vector<VariableName>>("v") = _coupled_v[i][j];
          _problem->addKernel("CoupledBEEquilibriumSub",
                              _solver_species[i] + "_" + _eq_species[j] + "_sub",
                              params_sub);
        }

        {
          InputParameters params_cd = _factory.getValidParams("CoupledDiffusionReactionSub");
          assignBlocks(params_cd, _blocks);
          params_cd.set<NonlinearVariableName>("variable") = _solver_species[i];
          params_cd.set<Real>("weight") = _weights[i][j];
          params_cd.defaultCoupledValue("log_k", _eq_const[j]);
          params_cd.set<Real>("sto_u") = _sto_u[i][j];
          params_cd.set<std::vector<Real>>("sto_v") = _sto_v[i][j];
          params_cd.set<std::vector<VariableName>>("v") = _coupled_v[i][j];
          _problem->addKernel("CoupledDiffusionReactionSub",
                              _solver_species[i] + "_" + _eq_species[j] + "_cd",
                              params_cd);
        }

        // If pressure is coupled, add a CoupledConvectionReactionSub Kernel as well
        if (getParam<bool>("add_darcy_advection_term") && isParamValid("pressure"))
        {
          InputParameters params_conv = _factory.getValidParams("CoupledConvectionReactionSub");
          assignBlocks(params_conv, _blocks);
          params_conv.set<NonlinearVariableName>("variable") = _solver_species[i];
          params_conv.set<Real>("weight") = _weights[i][j];
          params_conv.defaultCoupledValue("log_k", _eq_const[j]);
          params_conv.set<Real>("sto_u") = _sto_u[i][j];
          params_conv.set<std::vector<Real>>("sto_v") = _sto_v[i][j];
          params_conv.set<std::vector<VariableName>>("v") = _coupled_v[i][j];
          params_conv.set<std::vector<VariableName>>("p") = _pressure_var;
          params_conv.set<RealVectorValue>("gravity") = _gravity;
          _problem->addKernel("CoupledConvectionReactionSub",
                              _solver_species[i] + "_" + _eq_species[j] + "_conv",
                              params_conv);
        }
      }
    }
  }
}

void
AqueousReactionKinetics::addAuxiliaryVariables()
{
  // Add AqueousEquilibriumRxnAux AuxKernels for equilibrium species
  for (const auto j : make_range(_num_reactions))
  {
    // Add these aux-kernels only for the aux species involved in the reaction
    if (_aux_species.find(_eq_species[j]) != _aux_species.end())
    {
      InputParameters params_eq = _factory.getValidParams("AqueousEquilibriumRxnAux");
      assignBlocks(params, _blocks);
      params_eq.set<AuxVariableName>("variable") = _eq_species[j];
      params_eq.defaultCoupledValue("log_k", _eq_const[j]);
      params_eq.set<std::vector<Real>>("sto_v") = _stos[j];
      params_eq.set<std::vector<VariableName>>("v") = _solver_species_involved[j];
      getProblem().addAuxKernel("AqueousEquilibriumRxnAux", "aux_" + _eq_species[j], params_eq);
    }
  }
}
