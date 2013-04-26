/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "AddFieldSplitAction.h"
#include "FEProblem.h"

template<>
InputParameters validParams<AddFieldSplitAction>()
{
  InputParameters params =  validParams<Action>();
  params.addRequiredParam<std::string>("name", "FieldSplit name");
  params.addParam<std::vector<std::string> >("vars",   "Variables in this split (omitting this implies \"all variables\"");
  params.addParam<std::vector<std::string> >("blocks", "Mesh blocks in this split (omitting this implies \"all blocks\"");
  params.addParam<std::vector<std::string> >("sides",  "Sidesets in this split (omitting this implies \"no sidesets\"");
  params.addParam<std::vector<std::string> >("petsc_options", "PETSc flags for this split's solver");
  params.addParam<std::vector<std::string> >("petsc_options_iname", "PETSc option names for this split's solver");
  params.addParam<std::vector<std::string> >("petsc_options_value", "PETSc option values for this split's solver");
  params.addParam<std::vector<std::string> >("splits", "Names of subsplits (if any)");
  params.addParam<std::string>("fieldsplit_type", "additive", "FieldSplit decomposition type for the subsplits: additive|multiplicative|symmetric_multiplicative|schur");
  params.addParam<std::string>("schur_type", "full", "Type of Schur complement for the subsplits: full|upper|lower");
  params.addParam<std::string>("schur_pre",  "self", "Type of Schur complement preconditioner matrix for the subsplits: self|diag");
  return params;
}

AddFieldSplitAction::AddFieldSplitAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
AddFieldSplitAction::act()
{
  NonlinearSystem::FieldSplitInfo info;
  info.name = getParam<std::string>("name");
  if (!info.name.size()) {
    std::ostringstream err;
    err << "Empty split name is reserved.";
    mooseError(err.str());
  }
  info.vars   = getParam<std::vector<std::string> >("vars");
  info.blocks = getParam<std::vector<std::string> >("blocks");
  info.sides  = getParam<std::vector<std::string> >("sides");
  info.splits = getParam<std::vector<std::string> >("splits");
  info.fieldsplit_type = getParam<std::string>("fieldsplit_type");
  info.schur_type = getParam<std::string>("schur_type");
  info.schur_pre  = getParam<std::string>("schur_pre");
  info.petsc_options = getParam<std::vector<std::string> >("petsc_options");
  info.petsc_options_iname = getParam<std::vector<std::string> >("petsc_options_iname");
  info.petsc_options_value = getParam<std::vector<std::string> >("petsc_options_value");
  _problem->getNonlinearSystem().addFieldSplit(info.name, info);
}
